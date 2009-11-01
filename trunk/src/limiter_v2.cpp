/*
    Foo Lookahead Limiter - Lookahead limiter to keep peaks below an
                            user configured maximum. 2nd version
    Copyright (C) 2006  Sampo Savolainen <v2@iki.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <lv2plugin.hpp>
#include <cmath>
#include "utils.h"

using namespace LV2;
using namespace std;

#define PORT_INPUT_L		0
#define PORT_INPUT_R		1
#define PORT_OUTPUT_L		2
#define PORT_OUTPUT_R		3
#define PORT_INPUT_GAIN		4
#define PORT_MAX_PEAK_DB	5
#define PORT_ATTACK_MS		6
#define PORT_RELEASE_SECONDS	7
#define PORT_RELEASE_SCALE	8

#define PORT_ATTENUATION	9
#define PORT_LATENCY		10


#ifdef FOO_TESTER
#define debug(f, s...) printf(f, ## s)
#else
#define debug(f, s...)
#endif

#define FOO_LIMITER_MAX_ATTACK_MS	5.3f
#define FOO_LIMITER_DEFAULT_ATTACK_MS	3.0f

#define FOO_LIMITER_MAX_LOGSCALE	10.0f

typedef struct _CurvePoint {
	float gain;
	float delta;
} CurvePoint;

#define RINGBUFFER_SEEK(env, offset) ((env->ringbuffer_at+env->attack_length+(offset)) % (env->attack_length + 1))

#define RINGBUFFER_NEXT(env) RINGBUFFER_SEEK(env,  1)
#define RINGBUFFER_LAST(env) RINGBUFFER_SEEK(env, -1)


#define ENVELOPE_GAIN(env, offset) (env)->ringbuffer[ RINGBUFFER_SEEK((env), offset) ].gain

typedef struct _Envelope {
	// A ringbuffer of CurvePoints, size
	// is floor(samplerate * FOO_LIMITER_MAX_ATTACK_MS / 1000)  
	CurvePoint *ringbuffer; 
	int ringbuffer_at;

	

	// This envelope only moves forward, but "at" is needed to
	// calculate the log curve etc.
	int at;

	// attack_length also equals the amount of lookahead and
	// the used amount of the ringbuffer
	int attack_length;
	int release_length;

	// envelope_length = attack_length + release_length
	int envelope_length;

	// This is the target gain to reach when the limiter is 
	// at the precipice
	// This functions both as a target gain when ramping up
	// and it defines how the logarithmic release curve
	// is "scaled to fit"
	float limit_gain;


	// The logscale this envelope produces. Kept here to prevent
	// "zipper noise" from parameter changes while releasing
	float logscale;

} Envelope;



class LimiterV2 : public Plugin<LimiterV2>
{
public:
	LimiterV2(double rate)
		: Plugin<LimiterV2>(10)
	{
		samplerate = rate;

		max_attack_samples = (int)floor((float)samplerate * (float)FOO_LIMITER_MAX_ATTACK_MS / 1000.0f);

		/*
		//max_delta_change = 0.0001f; // TODO, calculate proper value...
		max_delta_change = 1000.0f;
		*/
		max_delta_change = 0.0002f;

		envelope.attack_length   = (int)floor((float)samplerate * FOO_LIMITER_DEFAULT_ATTACK_MS / 1000.0f);
		envelope.release_length  = 100; // just a number
		envelope.envelope_length = envelope.attack_length + envelope.release_length;
		envelope.at              = envelope.envelope_length + 1; // initial envelope is over
		envelope.limit_gain      = 0.0f; // just a number
		envelope.logscale        = 0.5f; // just another number

		envelope.ringbuffer_at   = 0;

		envelope.ringbuffer      = (CurvePoint *) malloc( (max_attack_samples + 1) * sizeof(CurvePoint));
		// Set the initial ringbuffer to all zeroes
		for (uint32_t i = 0; i < max_attack_samples + 1; i++) {
			envelope.ringbuffer[i].gain  = 1.0f;
			envelope.ringbuffer[i].delta = 0.0f;
		}


		// 4096 is just an estimate of a sane maximum buffer size. The limiter will
		// also work with bigger buffer sizes, it just has to divide processing into parts
		// (which it does internally)
		workbuffer_size  = 4096 + max_attack_samples;
		workbuffer_left  = new float[workbuffer_size];
		workbuffer_right = new float[workbuffer_size];

		memset(workbuffer_left,  0, sizeof(float) * workbuffer_size);
		memset(workbuffer_right, 0, sizeof(float) * workbuffer_size);

		debug("Instantiating FooLimiter v2, max_attack_samples = %d, current attack samples = %d\n",max_attack_samples, envelope.attack_length);


	}

	~LimiterV2()
	{
		delete workbuffer_left;
		delete workbuffer_right;
	}

	// The function uses the point in the ringbuffer at ( offset-1) to determine the
	// current change rate and works from there.
	// This function only works forward AND when the previous value has been calculated!!
	void calculateCurvePoint(Envelope *env, int offset, float max_delta_change)
	{
		// We'll ignore rate limiting for now, let's make the basics work for us first

		int at_envelope = env->at + offset;

		CurvePoint *prev_cpoint = &env->ringbuffer[RINGBUFFER_SEEK(env, offset-1)];
		CurvePoint *this_cpoint = &env->ringbuffer[RINGBUFFER_SEEK(env, offset)];
		float delta;

		// If the envelope is over. No rate limiting done here either, boring stuff
		if (at_envelope > env->envelope_length) {
			this_cpoint->gain  = 1.0f;
			this_cpoint->delta = 0.0f;

			return;
		} 

		// Attack phase
		if (at_envelope < env->attack_length) {

			// Here we would do rate limiting, but we won't.. yet

			int samples_to_precipice = env->attack_length - at_envelope;

			delta = (env->limit_gain - prev_cpoint->gain) / (float)(samples_to_precipice+1);
			if (delta > max_delta_change) {
				delta = max_delta_change;
			} else if (delta < -max_delta_change) {
				delta = -max_delta_change;
			}

			this_cpoint->delta = delta;

			this_cpoint->gain  = prev_cpoint->gain + delta;

			return;
		}

		// Release phase

		// calculate the point in the relese phase we are, result value = 0.0 .. 1.0
		float pos_release = (float)( at_envelope - env->attack_length) / (float)(env->release_length);
		//debug("pos = %f\n",pos_release);


		this_cpoint->delta = (1.0f - env->limit_gain)/env->logscale/( pos_release * expf(env->logscale)+1.0f-pos_release) / (float)(env->release_length);
		this_cpoint->gain  = env->limit_gain + ( (1.0f - env->limit_gain) * (logf(pos_release * expf(env->logscale) + 1.0f - (pos_release)) / env->logscale));
	}

	// We need a few envelope functions:
	//
	// restartEnvelope() builds a completely new envelope. This one is triggered when
	// The previous envelope is not running anymore or is at the release phase.
	//     It takes into account the previous curve delta and ramps up the attack accordingly
	//
	// extendEnvelope() extends the attack current envelope (which needs to be still ramping up).
	// Depending on the newly upcoming peak, the attack is either modified from this point onwards
	// OR it is extended after the current peak value by a less steep curve
	//
	// Both functions assume that they need to reach "limit_gain" in the either parametrized or current
	// attack_length samples

	void restartEnvelope(Envelope *env, int attack_samples, int release_samples, float limit_gain, float logscale, float max_delta_change)
	{
		int i;

		// Is the right one the previous delta, current delta or next delta?! who knows...
		CurvePoint prev_cpoint = env->ringbuffer[RINGBUFFER_LAST(env)];

		env->at = 0;

		env->attack_length   = attack_samples;
		debug("attack_length = %d\n",env->attack_length);
		env->release_length  = release_samples;
		env->envelope_length = attack_samples + release_samples;
		env->logscale        = 1.0f / expf(1.0f) + logscale * (FOO_LIMITER_MAX_LOGSCALE - 1.0f/expf(1.0f));

		env->limit_gain      = limit_gain;

		// We reset the ringbuffer start point.
		env->ringbuffer_at = 0;


		// Calculate "lookahead" (=attack_length) worth of envelope

		// We set the last position in the ringbuffer to the stored previous CurvePoint
		// so that calculateNextCurvePoint() will work
		env->ringbuffer[RINGBUFFER_LAST(env)] = prev_cpoint;

		for (i = 0; i < env->attack_length; i++) {
			calculateCurvePoint(env, i, max_delta_change);
		}
	}

	// Keeps the current attack time, as we are still in the attack phase. release time can be
	// changed without any problems.
	//
	// As we are still in the attack phase, it means that there are "env->at" samples of the previously
	// calculated release phase in the ringbuffer. So -env->at samples is the position of the previous
	// precipice. This function divides into two different parts:
	// A If the new limit_gain point requires a steeper slope than the old limit_gain
	//     We start ramping up the complete attack phase with the new delta
	// B If the new limit_gain point requires a shallower slope than the old limit_gain
	//     We continue on the current slope, but after reaching the original limit point, we start
	//     a secondary attack phase targeted to reach the newly found peak.
	//     We shall call this "intelligent sustain"
	//
	// Note I)
	// Correction regarding "steepness" of curve. In the limiter sense, the curve is "steepest" when
	// its' delta is lower than the others'. That is: 
	//   "Curve A is steeper than curve B, if A attenuates higher peaks than B"
	// So steepness in this limiter is about curves moving faster towards -inf.
	void extendEnvelope(Envelope *env, int release_samples, float new_limit_gain, float logscale, float max_delta_change)
	{
		int i;
		float current_ramp_delta, new_ramp_delta;

		CurvePoint prev_cpoint = env->ringbuffer[RINGBUFFER_LAST(env)];

		env->release_length  = release_samples;
		env->envelope_length = env->attack_length + release_samples;
		env->logscale        = 1.0f / expf(1.0f) + logscale * (FOO_LIMITER_MAX_LOGSCALE - 1.0f/expf(1.0f));

		// current ramp is: (current gain - limit gain) / samples left to current precipice
		current_ramp_delta = (env->limit_gain - prev_cpoint.gain) / (float)(env->attack_length - env->at);

		// the new ramp is: (current gain - new limit gain) / attack length
		new_ramp_delta     = (new_limit_gain - prev_cpoint.gain) / (float)(env->attack_length);

		debug(" comparing current_ramp_delta: %f, and new_ramp_delta: %f\n",current_ramp_delta, new_ramp_delta);

		// See "Note I" about delta comparison
		if (new_ramp_delta < current_ramp_delta) {
			// Case A

			debug(" * selected case A\n");

			env->limit_gain = new_limit_gain;
			env->at = 0;

			for (i = 0; i < env->attack_length; i++) {
				calculateCurvePoint(env, i, max_delta_change);
			}

		} else {
			// Case B, "intelligent sustain"

			i = (env->attack_length - env->at);
			env->at = 0;

#define INTELLIGENT_SUSTAIN

#ifndef INTELLIGENT_SUSTAIN

			debug(" * selected case B \"extend the current ramp\"\n");

			// We have "i" samples left.

			env->limit_gain += current_ramp_delta * (i);

#else
			// sounds good, but misses a few transients
			debug(" * selected case B \"intelligent sustain\"\n");


			// The curve up to the old precipice is already calculated,
			// If we set a new limit_gain and reset env->at, the
			// calculateNextCurvePoint() will do the "heavy lifting" as it
			// will automatically calculate the correct envelope for the
			// time between the old precipice and the new one

			// The time of the old precipice

			env->limit_gain = new_limit_gain;
#endif


			for (; i < env->attack_length; i++) {
				calculateCurvePoint(env, i, max_delta_change);
			}

		}

	}


	void run(uint32_t nframes)
	{

		
		uint32_t n,i,lookahead, buffer_offset;

		float min_gain = 1.0f;

		float peak_left, peak_right, current_gain;

		float input_gain     = DB_CO(*p(PORT_INPUT_GAIN));
		float max_peak       = DB_CO(*p(PORT_MAX_PEAK_DB));
		float attack_time_ms = *p(PORT_ATTACK_MS);
		float release_time   = *p(PORT_RELEASE_SECONDS);
		float release_scale  = *p(PORT_RELEASE_SCALE);

		float *input_left    = p(PORT_INPUT_L);
		float *input_right   = p(PORT_INPUT_R);
		float *output_left   = p(PORT_OUTPUT_L);
		float *output_right  = p(PORT_OUTPUT_R);

		uint32_t attack_length_parameter = (uint32_t)floor((float)samplerate * attack_time_ms / 1000.0f);
		buffer_offset = 0;

		// This was not enabled in the svn version. I wonder why? It's the only place where the users
		// new attack time will be taken into account
		if (envelope.at > envelope.envelope_length) {
			envelope.attack_length = attack_length_parameter;
			envelope.envelope_length = envelope.attack_length + envelope.release_length;
		}
		while (nframes > 0) {
			n = nframes;

			// Make sure we process in slices where "lookahead + slice" fit in the work buffer
			if (n > workbuffer_size - max_attack_samples)
				n = workbuffer_size - max_attack_samples;

			// The start of the workbuffer contains the "latent" samples from the previous run
			memcpy(workbuffer_left  + max_attack_samples, input_left  + buffer_offset, sizeof(float) * n);
			memcpy(workbuffer_right + max_attack_samples, input_right + buffer_offset, sizeof(float) * n);

			// _NEVER_ use the current attack length for peak detection. Use envelope.attack_length !!!!
			// TODO: as the attack time can be modified, this is going to be tricky...
			lookahead = envelope.attack_length;

			for (i = 0; i < n; i++, lookahead++) {

				// 1. input gain is applied to the work buffer

				workbuffer_left [lookahead] *= input_gain;
				workbuffer_right[lookahead] *= input_gain;

				// 2. Peak detection. 
				// Done before determining limiter gain because a new peak will have to affect
				// the envelope at /this/ point

				peak_left  = fabs(workbuffer_left [lookahead]);
				peak_right = fabs(workbuffer_right[lookahead]);

				// peak_left will contain the highest peak of the two
				if (peak_right > peak_left) peak_left = peak_right;

				float lookahead_gain = ENVELOPE_GAIN(&envelope, envelope.attack_length -1);

				if ( (peak_left * lookahead_gain) > max_peak) {
					// a peak which isn't contained by the limiter has been found!

					int release_samples  = (int)floor( release_time * (float)samplerate);
					debug("Found peak (%f, max = %f) at %lu, release_samples = %d\n", peak_left, max_peak, i, release_samples);
					float new_limit_gain = max_peak / peak_left;


					// two cases, if the current envelope is in the attack phase, we call
					// extendEnvelope

					if (envelope.at < envelope.attack_length) {
						extendEnvelope(&envelope, release_samples, new_limit_gain, release_scale, max_delta_change);
					} else {
						restartEnvelope(&envelope, envelope.attack_length, release_samples, new_limit_gain, release_scale, max_delta_change);
						/*
						   FooLimiter2_restartEnvelope(&envelope, attack_length_parameter, release_samples, new_limit_gain, release_scale, max_delta_change);

						// We might miss a few transients, but we don't care
						debug("lookahead was %lu", lookahead);
						lookahead = i + envelope.attack_length;
						debug(", lookahead is %lu\n",lookahead);
						*/
					}
					// we have to twiddle the envelope

				}




				// 3. Determine and apply limiter gain

				current_gain = ENVELOPE_GAIN(&envelope, 0);

				if (current_gain < min_gain)
					min_gain = current_gain;

				// 4. Brickwall limiter stage, clip whatever goes over

				float out_left  = workbuffer_left[i] * current_gain;
				float out_right = workbuffer_right[i] * current_gain;

#define BRICKWALL(param, limit) if (param > limit) { param = limit; } else if (param < -limit) { param = -limit; }

				BRICKWALL(out_left,  max_peak);
				BRICKWALL(out_right, max_peak);

				output_left [i+buffer_offset] = out_left;


#ifndef FOO_TESTER
				output_right[i+buffer_offset] = out_right;
#else
				// when debugging, write the output envelope to the right channel
				output_right[i+buffer_offset] = current_gain;
#endif


				// 4. Advance envelope

				calculateCurvePoint(&envelope,envelope.attack_length, max_delta_change);
				if (envelope.at <= envelope.envelope_length)
					envelope.at++;
				envelope.ringbuffer_at = (envelope.ringbuffer_at + 1) % (envelope.attack_length + 1);

			}

			// copy the "end" of input data to the lookahead buffer
			// copy input[ (n-max_attack_samples] .. n ] => lookahead

			if (n < max_attack_samples) {
				memmove(workbuffer_left,  workbuffer_left  + n , sizeof(float) * max_attack_samples);
				memmove(workbuffer_right, workbuffer_right + n , sizeof(float) * max_attack_samples);
			} else {
				memcpy (workbuffer_left,  workbuffer_left  + n , sizeof(float) * max_attack_samples);
				memcpy (workbuffer_right, workbuffer_right + n , sizeof(float) * max_attack_samples);
			}

			nframes -= n;
			buffer_offset += n;
		}

		*p(PORT_ATTENUATION) = -CO_DB(min_gain);
		*p(PORT_LATENCY)     = max_attack_samples;
	}


private:

	unsigned long samplerate;
	uint32_t max_attack_samples;
	float max_delta_change;

	Envelope envelope;

	uint32_t workbuffer_size;
	float *workbuffer_left;
	float *workbuffer_right;


};

static int _ = LimiterV2::register_class("http://studionumbersix.com/foo/lv2/limiter-v2");


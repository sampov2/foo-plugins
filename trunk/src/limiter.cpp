/*
    Foo Lookahead Limiter - Lookahead limiter to keep peaks below a set threshold
    Copyright (C) 2006-2009  Sampo Savolainen <v2@iki.fi>

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
#define PORT_RELEASE_TIME	6
#define PORT_RELEASE_SCALE	7

#define PORT_ATTENUATION	8
#define PORT_LATENCY		9



#define FOO_LIMITER_RAMP_UP_MILLISECONDS 2.5f

#define FOO_LIMITER_MAX_LOGSCALE 10.0f

typedef struct _Envelope {
	uint32_t ramp_samples;
	uint32_t release_samples;
	uint32_t length_samples; // ramp_samples + sustain_samples + release_samples

	float start_gain;
	float limit_gain;

	// ramp_delta = (env->limit_gain - env->start_gain)/(float)env->ramp_samples
	float ramp_delta;

	// release_delta = (1.0f - env->limit_gain)/(float)env->release_samples
	float release_delta;

	uint32_t at_sample; 

	// Logarithmic release envelope
	float logscale;
} Envelope;


class Limiter : public Plugin<Limiter>
{
public:
	Limiter(double rate)
		: Plugin<Limiter>(10)
	{
		samplerate = rate;
		ramp_up_time_samples = (uint32_t)floor(rate * (float)FOO_LIMITER_RAMP_UP_MILLISECONDS / 1000.0f);

		workbuffer_size  = 4096 + ramp_up_time_samples;
		workbuffer_left  = new float[workbuffer_size];
		workbuffer_right = new float[workbuffer_size];

		memset(workbuffer_left,  0, sizeof(float) * workbuffer_size);
		memset(workbuffer_right, 0, sizeof(float) * workbuffer_size);

		current_gain = 1.0f;

		// create dummy envelope which has already been passed
		// make sure triggerEnvelope doesn't use uninitialized values
		envelope.at_sample       = -1;
		envelope.ramp_samples    = 1;
		envelope.ramp_delta      = 0.0f;
		envelope.release_samples = 1;
		triggerEnvelope(&envelope, 1, 1, 1.0f, 1.0f);
		envelope.at_sample       = 3;
	}

	~Limiter()
	{
		delete workbuffer_left;
		delete workbuffer_right;
	}

 	void inline triggerEnvelope(Envelope *env, uint32_t ramp_samples, uint32_t release_samples,
				    float new_limit_gain, float logscale)
	{
		float new_ramp_delta = (new_limit_gain - current_gain)/(float)ramp_samples;

		// New envelopes are of suspect if previous envelope is still ramping up
		if (env->at_sample < env->ramp_samples) {

			// the new ramp delta is not as steep as the old one
			if (new_ramp_delta > env->ramp_delta) {
				// instead of creating a new envelope, we extend the current one
				env->at_sample = 0;
				env->start_gain = current_gain;
				env->limit_gain = current_gain + env->ramp_delta * (float)env->ramp_samples;
				env->release_delta = (1.0f - env->limit_gain)/(float)env->release_samples;

				// Logscale parameter will not be changed during ramp-up as it could
				// cause peaks to escape
				return;
			}

		}

		env->ramp_samples    = ramp_samples;
		env->release_samples = release_samples;
		env->length_samples  = env->ramp_samples + env->release_samples;

		env->start_gain      = current_gain;
		env->limit_gain      = new_limit_gain;

		env->ramp_delta      = (env->limit_gain - env->start_gain)/(float)env->ramp_samples;
		env->release_delta   = (1.0f - env->limit_gain)/(float)env->release_samples;

		env->logscale        = 1 / expf(1.0f) + logscale * (FOO_LIMITER_MAX_LOGSCALE - 1/expf(1.0f));

		env->at_sample       = 0;
	}

#define LOGSCALE(position, scale)	( logf( (position) * exp(scale) + 1.0f - (position)) / (scale))

	float calculateEnvelope(Envelope *env, uint32_t offset)
	{
		uint32_t at = env->at_sample + offset;

		if (at >= env->length_samples) return 1.0f;
		if (at <  0) return env->start_gain;

		// RAMP
		if (at <  env->ramp_samples) {
			return env->start_gain + (float)at * env->ramp_delta;
		}

		// RELEASE
		return env->limit_gain + ( (1.0f - env->limit_gain) * LOGSCALE( ((float) (at - env->ramp_samples))/(float)(env->release_samples), env->logscale));
	}

	void run(uint32_t nframes)
	{

		uint32_t n,i,lookahead, buffer_offset;
		float min_gain = 1.0f;

		float peak_left, peak_right, tmp;

		float *input_left   = p(PORT_INPUT_L);
		float *input_right  = p(PORT_INPUT_R);
		float *output_left  = p(PORT_OUTPUT_L);
		float *output_right = p(PORT_OUTPUT_R);

		float max_peak      = DB_CO(*p(PORT_MAX_PEAK_DB));
		float input_gain    = DB_CO(*p(PORT_INPUT_GAIN));
		float release_time  = *p(PORT_RELEASE_TIME);
		float release_scale = *p(PORT_RELEASE_SCALE);


		buffer_offset = 0;

		while (nframes > 0) {
			n = nframes;

			// Make sure we process in slices where "lookahead + slice" fit in the work buffer
			if (n > workbuffer_size - ramp_up_time_samples)
				n = workbuffer_size - ramp_up_time_samples;

			// The start of the workbuffer contains the "latent" samples from the previous run
			memcpy(workbuffer_left  + ramp_up_time_samples, input_left  + buffer_offset, sizeof(float) * n);
			memcpy(workbuffer_right + ramp_up_time_samples, input_right + buffer_offset, sizeof(float) * n);

			lookahead = ramp_up_time_samples;
			for (i = 0; i < n; i++, lookahead++) {

				workbuffer_left [lookahead] *= input_gain;
				workbuffer_right[lookahead] *= input_gain;

				current_gain = calculateEnvelope(&envelope, 0);

				// Peak detection
				peak_left  = fabsf(workbuffer_left [lookahead]);
				peak_right = fabsf(workbuffer_right[lookahead]);

				tmp = fmaxf(peak_left, peak_right) * calculateEnvelope(&envelope, ramp_up_time_samples);

				if (tmp > max_peak) {

					// calculate needed_limit_gain
					float needed_limit_gain = max_peak / fmaxf(peak_left, peak_right);

					uint32_t release_samples = (uint32_t)floor( release_time * (float)samplerate);

					triggerEnvelope(&envelope, ramp_up_time_samples, release_samples, needed_limit_gain, release_scale);
				}

				if (current_gain < min_gain) min_gain = current_gain;

				if (envelope.at_sample <= envelope.length_samples)
					envelope.at_sample++;


				output_left [i+buffer_offset] = workbuffer_left [i] * current_gain;
				output_right[i+buffer_offset] = workbuffer_right[i] * current_gain;
			}

			// copy the "end" of input data to the lookahead buffer
			// copy input[ (n-ramp_up_time_samples] .. n ] => lookahead

			if (n < ramp_up_time_samples) {
				memmove(workbuffer_left,  workbuffer_left  + n , sizeof(float) * ramp_up_time_samples);
				memmove(workbuffer_right, workbuffer_right + n , sizeof(float) * ramp_up_time_samples);
			} else {
				memcpy (workbuffer_left,  workbuffer_left  + n , sizeof(float) * ramp_up_time_samples);
				memcpy (workbuffer_right, workbuffer_right + n , sizeof(float) * ramp_up_time_samples);
			}

			nframes -= n;
			buffer_offset += n;
		}

		*p(PORT_ATTENUATION) = -CO_DB(min_gain);
		*p(PORT_LATENCY) = ramp_up_time_samples;
	}

private:
	uint32_t ramp_up_time_samples;
	uint32_t samplerate;

	float current_gain;
	Envelope envelope;

	uint32_t workbuffer_size;
	float *workbuffer_left;
	float *workbuffer_right;

};

static int _ = Limiter::register_class("http://studionumbersix.com/foo/lv2/limiter");


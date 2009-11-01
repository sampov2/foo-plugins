/*
    t00b limiter - research limiter into the world of emulating hardware
    Copyright (C) 2006-2009  Sampo Savolainen <v2@iki.fi>
    Many thanks to Nick Mainsbridge

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
#include "rms.h"

using namespace LV2;
using namespace std;

using Foo::RMS;

#define PORT_INPUT_L            0
#define PORT_INPUT_R            1
#define PORT_OUTPUT_L           2
#define PORT_OUTPUT_R           3
#define PORT_INPUT_GAIN         4
#define PORT_MAX_PEAK_DB        5
#define PORT_ATTACK_SECONDS     6
#define PORT_RELEASE_SECONDS    7

#define PORT_ATTENUATION        8

class t00bLimiter : public Plugin<t00bLimiter>
{
public:
	t00bLimiter(double rate)
		: Plugin<t00bLimiter>(9),
		  rms_l(rate, 0.002f),
		  rms_r(rate, 0.002f)
	{
		samplerate = rate;

		current_gain = 1.0f;
		tangent = 0.0f;
	}

	void run(uint32_t nframes)
	{
		// TODO: make it work, add input gain, etc. etc.
		float max_peak   = DB_CO(*p(PORT_MAX_PEAK_DB));

		float attack  = (float)samplerate * *p(PORT_ATTACK_SECONDS);
		float release = (float)samplerate * *p(PORT_RELEASE_SECONDS);

		float *input_left   = p(PORT_INPUT_L);
		float *input_right  = p(PORT_INPUT_R);
		float *output_left  = p(PORT_OUTPUT_L);
		float *output_right = p(PORT_OUTPUT_R);

		//attack = 1.5f;

		//printf("%f, %f\n",attack, release);

		for (uint32_t n = 0; n < nframes; ++n) {

			float rms  = run_RMS (rms_l, rms_r, input_left[n], input_right[n]);
			float peak = run_peak(input_left[n], input_right[n]);

			saturation = optoSaturator(attack, release, saturation, peak);


			float target_gain;

			if (saturation > max_peak) {
				// no limiting yet, let's see that gain calculation goes right
				target_gain = max_peak / saturation;
			} else {
				target_gain = 1.0f;
			}

			float old_gain = current_gain;

			current_gain = limitSlewrate(0.005f, &tangent, old_gain, target_gain);
			//current_gain = target_gain;


			output_left[n]  = input_left[n]  * current_gain;
			output_right[n] = input_right[n] * current_gain;
			//buffer_write(output_left[n],  tangent);
			//buffer_write(output_left[n], tangent);

			//buffer_write(output_right[n], current_gain);


		}



	}


private:
	inline float run_RMS(RMS &rms_l, RMS &rms_r, float input_l, float input_r)
	{
		float current_l = rms_l.run(input_l);
		float current_r = rms_r.run(input_r);

		return fmaxf(current_l, current_r);
	}

	inline float run_peak(float input_l, float input_r)
	{
		return fmaxf(fabsf(input_l), fabsf(input_r));
	}

	 inline float optoSaturator(float attackResistance, float releaseResistance, float saturation, float peak)
	 {
		 // attack
		 if (peak > saturation) {
			 saturation += (peak-saturation)/attackResistance;
		 } else {
			 saturation -= (saturation-peak)/releaseResistance;
		 }

		 return saturation;
	 }

	inline float limitSlewrate(float max_slew_rate, float *prev_tangent, float prev_value, float new_value)
	{
		float target_tangent = new_value - prev_value;

		if (target_tangent < *prev_tangent) {
			if (target_tangent < *prev_tangent - max_slew_rate) {
				target_tangent = *prev_tangent - max_slew_rate;
				new_value = prev_value + target_tangent;
			}
		} else {
			if (target_tangent > *prev_tangent + max_slew_rate) {
				target_tangent = *prev_tangent + max_slew_rate;
				new_value = prev_value + target_tangent;
			}
		}

		*prev_tangent = target_tangent;

		return new_value;
	}

	float current_gain;
	float tangent;
	float saturation;
	RMS rms_l;
	RMS rms_r;

	unsigned long samplerate;
};

static int _ = t00bLimiter::register_class("http://studionumbersix.com/foo/lv2/t00b-limiter");


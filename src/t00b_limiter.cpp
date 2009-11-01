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

/*
<ladspa>
  <global>
    <meta name="maker" value="Sampo Savolainen &lt;v2@iki.fi&gt;"/>
    <meta name="copyright" value="GPL"/>
    <meta name="properties" value="HARD_RT_CAPABLE"/>
    <code><![CDATA[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rms.h"


static inline float run_RMS(rms *rms_l, rms *rms_r, float input_l, float input_r)
{
	float current_l = rms_run(rms_l, input_l);
	float current_r = rms_run(rms_r, input_r);

	return fmaxf(current_l, current_r);
}

static inline float run_peak(float input_l, float input_r)
{
	return fmaxf(fabsf(input_l), fabsf(input_r));
}

static inline float optoSaturator(float attackResistance, float releaseResistance, float saturation, float peak)
{
	// attack
	if (peak > saturation) {
		saturation += (peak-saturation)/attackResistance;
	} else {
		saturation -= (saturation-peak)/releaseResistance;
	}

	return saturation;
}

static inline float limitSlewrate(float max_slew_rate, float *prev_tangent, float prev_value, float new_value)
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




    ]]></code>
  </global>

  <plugin label="t00b_limiter" id="3186" class="LimiterPlugin">
    <name>t00b Limiter</name>


    <callback event="instantiate"><![CDATA[
	samplerate = s_rate;

	rms_l = rms_new((float)samplerate,0.002f);
	rms_r = rms_new((float)samplerate,0.002f);
	current_gain = 1.0f;
	tangent = 0.0f;
    ]]></callback>

    <callback event="cleanup"><![CDATA[
    	rms_free(plugin_data->rms_l);
    	rms_free(plugin_data->rms_r);

    ]]></callback>

    <callback event="run"><![CDATA[
	long n;
	float max_peak   = DB_CO(max_peak_db);

	float attack  = (float)samplerate * attack_time;
	float release = (float)samplerate * release_time;

	//attack = 1.5f;

	//printf("%f, %f\n",attack, release);

	for (n = 0; n < sample_count; n++) {

		float rms  = run_RMS (plugin_data->rms_l, plugin_data->rms_r, 
		                      input_left[n], input_right[n]);

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


		buffer_write(output_left[n],  input_left[n]  * current_gain);
		buffer_write(output_right[n], input_right[n] * current_gain);
		//buffer_write(output_left[n],  tangent);
		//buffer_write(output_left[n], tangent);

		buffer_write(output_right[n], current_gain);


	}

    	

    ]]></callback>


<!-- Do this later
    <port label="input_gain_db" dir="input" type="control" hint="default_0">
      <name>Input gain (dB)</name>
      <p>Maximum peak level to limit</p>
      <range min="-20.0" max="+10.0"/>
    </port>
-->
    <port label="max_peak_db" dir="input" type="control" hint="default_0">
      <name>Max level (dB)</name>
      <p>Maximum peak level to limit</p>
      <range min="-30.0" max="+0.0"/>
    </port>

    <port label="attack_time" dir="input" type="control" hint="default_low">
      <name>Attack time (s)</name>
      <p>Limiter release time in milliseconds</p>
      <range min="0.001" max="0.2"/>
    </port>

    <port label="release_time" dir="input" type="control" hint="default_low">
      <name>Release time (s)</name>
      <p>Limiter release time in milliseconds</p>
      <range min="0.01" max="2.0"/>
    </port>

    <port label="attenuation" dir="output" type="control">
      <name>Attenuation (dB)</name>
      <p>The amount of attenuation at the moment</p>
      <range min="0.0" max="+70.0"/>
    </port>


    <port label="input_left" dir="input" type="audio">
      <name>Input L</name>
    </port>

    <port label="input_right" dir="input" type="audio">
      <name>Input R</name>
    </port>

    <port label="output_left" dir="output" type="audio">
      <name>Output L</name>
    </port>

    <port label="output_right" dir="output" type="audio">
      <name>Output R</name>
    </port>

    <instance-data label="current_gain" type="float"/>
    <instance-data label="tangent" type="float"/>
    <instance-data label="saturation" type="float"/>
    <instance-data label="rms_l" type="rms *"/>
    <instance-data label="rms_r" type="rms *"/>

    <instance-data label="samplerate" type="unsigned long"/>

   <code><![CDATA[
	]]>
    </code>

  </plugin>
 
</ladspa>
*/

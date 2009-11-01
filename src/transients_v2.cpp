/*
    Foo Transient Architect - control relative dynamics 
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
#include "rms.h"

using namespace LV2;
using namespace std;

#define PORT_INPUT_L            0
#define PORT_INPUT_R            1
#define PORT_OUTPUT_L           2
#define PORT_OUTPUT_R           3
#define PORT_SMOOTH_DRASTIC	4
#define PORT_ATTACK_EMPHASIS	5
#define PORT_RELEASE_EMPHASIS	6
#define PORT_ACTUAL_GAIN	7

class TransientsV2 : public Plugin<TransientsV2>
{
public:
	TransientsV2(double rate)
		: Plugin<TransientsV2>(8),
		  slow_rms(rate, 1.0f),
		  fast_rms(rate, 1.0f)
	{
	}
	
	void run(uint32_t nframes)
	{
		float *input_left	      = p(PORT_INPUT_L);
		float *input_right	      = p(PORT_INPUT_R);
		float *output_left	      = p(PORT_OUTPUT_L);
		float *output_right	      = p(PORT_OUTPUT_R);
		float smooth_vs_drastic       = *p(PORT_SMOOTH_DRASTIC);
		float attack_gain_multiplier  = *p(PORT_ATTACK_EMPHASIS);
		float release_gain_multiplier = *p(PORT_RELEASE_EMPHASIS);
		float tmp, fast, slow;

#define MIN_ENVELOPE_SPEED 0.01f
#define MAX_ENVELOPE_SPEED 0.06f

		float average_difference = MIN_ENVELOPE_SPEED + smooth_vs_drastic * (MAX_ENVELOPE_SPEED - MIN_ENVELOPE_SPEED);


		slow_rms.set_time(0.05f + average_difference / 2.0f);
		fast_rms.set_time(0.05f - average_difference / 2.0f);

		float max_gain = 0.0;
		float min_gain = 0.0;
		float current_gain;


		for (uint32_t i = 0; i < nframes; i++) {

			if (fabsf(input_left[i]) > fabsf(input_right[i]))
				tmp = input_left[i];
			else
				tmp = input_right[i];

			slow = CO_DB(slow_rms.run(tmp));
			fast = CO_DB(fast_rms.run(tmp));

			if (fast > slow) {
				// Attack
				tmp = (fast - slow) * attack_gain_multiplier;
			} else {
				// Release
				tmp = (slow - fast) * release_gain_multiplier;
			}

			//printf("current_gain = %f * (1.0f - %f) + %f * %f\n",current_gain, gain_smoothing, DB_CO(tmp), gain_smoothing);

			current_gain = DB_CO(tmp);

			if (tmp > max_gain) max_gain = tmp;
			if (tmp < min_gain) min_gain = tmp;

			output_left[i]  = input_left[i]  * current_gain;
			output_right[i] = input_right[i] * current_gain;

#ifdef FOO_TESTER
			// Default tester operations, suitable for architect_graph.sh
			output_left[i]  = fast;
			output_right[i] = slow;
#endif 
		}

		if (max_gain > -min_gain) 
			*p(PORT_ACTUAL_GAIN) = max_gain;
		else
			*p(PORT_ACTUAL_GAIN) = min_gain;


	}

private:
    	Foo::RMS slow_rms;
	Foo::RMS fast_rms;
};

static int _ = TransientsV2::register_class("http://studionumbersix.com/foo/lv2/transients-v2");


/*
<ladspa>
  <global>
    <meta name="maker" value="Sampo Savolainen &lt;v2@iki.fi&gt;"/>
    <meta name="copyright" value="GPL"/>
    <meta name="properties" value="HARD_RT_CAPABLE"/>
  </global>

  <plugin label="foo_transients_v2" id="3189" class="DynamicsPlugin">
    <name>Foo Transient Architect v2</name>

    <callback event="instantiate"><![CDATA[
	slow_rms = rms_new((float)s_rate, 1.0f);
        fast_rms = rms_new((float)s_rate, 1.0f);
    ]]></callback>

    <callback event="cleanup"><![CDATA[
	rms_free (plugin_data->fast_rms);
	rms_free (plugin_data->slow_rms);
    ]]></callback>

    <callback event="run"><![CDATA[
	// DSP
	unsigned long i;
	float tmp, fast, slow;

#define MIN_ENVELOPE_SPEED 0.01f
#define MAX_ENVELOPE_SPEED 0.06f

	float average_difference = MIN_ENVELOPE_SPEED + smooth_vs_drastic * (MAX_ENVELOPE_SPEED - MIN_ENVELOPE_SPEED);


	rms_set_time(slow_rms, 0.05f + average_difference / 2.0f);
	rms_set_time(fast_rms, 0.05f - average_difference / 2.0f);

	float max_gain = 0.0;
	float min_gain = 0.0;
	float current_gain;


	for (i = 0; i < sample_count; i++) {

	    if (fabsf(input_left[i]) > fabsf(input_right[i]))
	        tmp = input_left[i];
	    else
	        tmp = input_right[i];

	    slow = CO_DB(rms_run(slow_rms, tmp));
	    fast = CO_DB(rms_run(fast_rms, tmp));

	    if (fast > slow) {
		// Attack
		tmp = (fast - slow) * attack_gain_multiplier;
	    } else {
		// Release
		tmp = (slow - fast) * release_gain_multiplier;
	    }

	    //printf("current_gain = %f * (1.0f - %f) + %f * %f\n",current_gain, gain_smoothing, DB_CO(tmp), gain_smoothing);

	    current_gain = DB_CO(tmp);

	    if (tmp > max_gain) max_gain = tmp;
	    if (tmp < min_gain) min_gain = tmp;

	    buffer_write(output_left[i],  input_left[i]  * current_gain);
	    buffer_write(output_right[i], input_right[i] * current_gain);

#ifdef FOO_TESTER
	    // Default tester operations, suitable for architect_graph.sh
	    buffer_write(output_left[i], fast);
	    buffer_write(output_right[i], slow);
#endif 
	}

	if (max_gain > -min_gain) 
		*(plugin_data->actual_gain) = max_gain;
	else
		*(plugin_data->actual_gain) = min_gain;

    ]]></callback>


    <port label="smooth_vs_drastic" dir="input" type="control" hint="default_high">
      <name>Smooth vs Drastic</name>
      <p>Controls the averaging time difference between the controlling envelopes</p>
      <range min="0.0" max="1.0"/>
    </port>

    <port label="attack_gain_multiplier" dir="input" type="control" hint="default_0">
      <name>Attack control</name>
      <p>Attack gain range control</p>
      <range min="-10.0" max="+10.0"/>
    </port>

    <port label="release_gain_multiplier" dir="input" type="control" hint="default_0">
      <name>Release control</name>
      <p>Release gain range control</p>
      <range min="-10.0" max="+10.0"/>
    </port>

    <port label="actual_gain" dir="output" type="control">
      <name>Current gain (dB)</name>
      <p>The amount of gain applied at the moment</p>
      <range min="-30.0" max="+30.0"/>
    </port>

    <port label="latency" dir="output" type="control">
      <name>latency</name>
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

    <instance-data label="slow_rms"     type="rms *"/>
    <instance-data label="fast_rms"     type="rms *"/>

  </plugin>
</ladspa>
*/

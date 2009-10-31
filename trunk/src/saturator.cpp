/*
    Foo Saturator - versatile saturator with drive
    Copyright (C) 2008-2009  Sampo Savolainen <v2@iki.fi>

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

#define PORT_INPUT              0
#define PORT_OUTPUT             1
#define PORT_INPUT_GAIN		2
#define PORT_MAX_PEAK_DB	3
#define PORT_SQUAREWAVENESS	4
#define PORT_DRIVE		5

/*
	 Signal flow
	   input
	     |
	  detector ->  CV
	     |          |
	    VCA    <- Slew rate limiter
	     |
	   output
*/

#define PARAMETER_SLOPE_SAMPLES_SECONDS 0.0025

typedef struct SaturatorParameters_
{
	float gain;
	float peak;
	float depth;
	float drive;

	float threshold; // not parametrized
} SaturatorParameters; 


class Saturator : public Plugin<Saturator>
{
public:
	Saturator(double rate)
		: Plugin<Saturator>(6)
	{
		samplerate = rate;

		last_parameters.gain = 1.0;
		last_parameters.peak = 1.0;
		last_parameters.depth = 0.3;
		last_parameters.drive = 1.0;

		last_parameters.threshold = 1.0 - 0.3;
	}


	void run(uint32_t nframes)
	{
		SaturatorParameters curr_p, new_parameters;
		uint32_t slope_length, i;
		float t, input, sign, CV = 1.0f;

		float *input_buf  = p(PORT_INPUT);
		float *output_buf = p(PORT_OUTPUT);

		float square  = *p(PORT_SQUAREWAVENESS);
		float drive   = *p(PORT_DRIVE);

		new_parameters.gain  = DB_CO(*p(PORT_INPUT_GAIN));
		new_parameters.peak  = DB_CO(*p(PORT_MAX_PEAK_DB));
		new_parameters.depth = (1.0f - square) * new_parameters.peak;
		new_parameters.drive = drive;
		new_parameters.threshold = new_parameters.peak - new_parameters.depth;


		//printf("Bo: gain %f, peak %f, depth %f, drive %f\n",new_parameters.gain, new_parameters.peak, new_parameters.depth, new_parameters.drive);

		/*
		   printf("gain %f - %f, peak %f - %f, depth %f - %f, drive %f - %f\n",
		   last_parameters.gain,  new_parameters.gain,
		   last_parameters.peak,  new_parameters.peak,
		   last_parameters.depth, new_parameters.depth,
		   last_parameters.drive, new_parameters.drive);
		   */

		// threshold is calculated, so no need to check it
		if (( last_parameters.gain  == new_parameters.gain  &&
		      last_parameters.peak  == new_parameters.peak  &&
		      last_parameters.depth == new_parameters.depth &&
		      last_parameters.drive == new_parameters.drive) ) {
			slope_length = 0;
		} else {
			slope_length = samplerate * PARAMETER_SLOPE_SAMPLES_SECONDS;
			if ( (nframes-1) < slope_length) {
				slope_length = nframes-1;
			}
		}

		float ratio;

		/*
		   if (slope_length > 0) {
		   printf("Changing parameters %d %d\n", sample_count, slope_length);
		   }
		   */

		for (i = 0; i < nframes; i++) {
			if (i < slope_length) {
				ratio = (float)(i+1)/(float)(slope_length+1);
				curr_p.gain  = (1.0 - ratio) * last_parameters.gain  + ratio * new_parameters.gain;
				curr_p.peak  = (1.0 - ratio) * last_parameters.peak  + ratio * new_parameters.peak;
				curr_p.depth = (1.0 - ratio) * last_parameters.depth + ratio * new_parameters.depth;
				curr_p.drive = (1.0 - ratio) * last_parameters.drive + ratio * new_parameters.drive;

				curr_p.threshold = curr_p.peak - curr_p.depth;
			} else {
				curr_p = new_parameters;
				curr_p.threshold = curr_p.peak - curr_p.depth;
			}


			input = input_buf[i] * curr_p.gain;
			sign = (input < 0.0 ? -1.0 : 1.0);
			t = fabsf(input);

			// detector
			if (t <= curr_p.threshold) {
				CV = 1.0f;
			} else {
				// CV

				// remove the threshold
				t -= curr_p.threshold;

				// scale to the area
				t /= curr_p.depth;

				// apply drive
				t *= curr_p.drive;

				// tan it
				t  = tanh(t);

				// create target sample
				t = (curr_p.threshold + t*curr_p.depth) * sign; // scale it back

				// targetCV = the gain required to apply to input to reach t
				CV = t / input;
			}

			output_buf[i] = input_buf[i] * curr_p.gain * CV;

		}

		last_parameters = new_parameters;


	}
private:
	SaturatorParameters last_parameters;
    	double samplerate;
	
};

static int _ = Saturator::register_class("http://studionumbersix.com/foo/lv2/saturator");

/*

<ladspa>
  <global>
    <meta name="maker" value="Sampo Savolainen &lt;v2@iki.fi&gt;"/>
    <meta name="copyright" value="GPL"/>
    <meta name="properties" value="HARD_RT_CAPABLE"/>
    <code><![CDATA[

#define PARAMETER_SLOPE_SAMPLES_SECONDS 0.0025

typedef struct SaturatorParameters_
{
	float gain;
	float peak;
	float depth;
	float drive;

	float threshold; // not parametrized
} SaturatorParameters; 
    ]]></code>
  </global>

  <plugin label="foo_saturator" id="3187" class="DistortionPlugin">
    <name>Foo Saturator</name>


    <callback event="instantiate"><![CDATA[
	samplerate = (float)s_rate;

	//printf("instantiate saturator\n");

	last_parameters.gain = 1.0;
	last_parameters.peak = 1.0;
	last_parameters.depth = 0.3;
	last_parameters.drive = 1.0;
	
	last_parameters.threshold = 1.0 - 0.3;
    ]]></callback>

	<!--
	 Signal flow
	   input
	     |
	  detector ->  CV
	     |          |
	    VCA    <- Slew rate limiter
	     |
	   output
	-->

    <callback event="run"><![CDATA[
	SaturatorParameters p, new_parameters;
	int slope_length, i;
	float t, input, sign, CV = 1.0f;

	new_parameters.gain  = DB_CO(input_gain_db); // TODO: db -> coef conversion
	new_parameters.peak  = DB_CO(max_peak_db);   // ditto
	new_parameters.depth = (1.0f - square) * new_parameters.peak;
	new_parameters.drive = drive;
	new_parameters.threshold = new_parameters.peak - new_parameters.depth;

	*(plugin_data->latency) = 0;
	//latency = 0;

	//printf("Bo: gain %f, peak %f, depth %f, drive %f\n",new_parameters.gain, new_parameters.peak, new_parameters.depth, new_parameters.drive);

	//printf("gain %f - %f, peak %f - %f, depth %f - %f, drive %f - %f\n",
	//      last_parameters.gain,  new_parameters.gain,
	//      last_parameters.peak,  new_parameters.peak,
	//      last_parameters.depth, new_parameters.depth,
	//      last_parameters.drive, new_parameters.drive);

	// threshold is calculated, so no need to check it
	if (( last_parameters.gain  == new_parameters.gain  &&
	      last_parameters.peak  == new_parameters.peak  &&
	      last_parameters.depth == new_parameters.depth &&
	      last_parameters.drive == new_parameters.drive) ) {
		slope_length = 0;
	} else {
		slope_length = samplerate * PARAMETER_SLOPE_SAMPLES_SECONDS;
		if ( (sample_count-1) < slope_length) {
			slope_length = sample_count-1;
		}
	}

	float ratio;

	//if (slope_length > 0) {
	//	printf("Changing parameters %d %d\n", sample_count, slope_length);
	//}

	for (i = 0; i < sample_count; i++) {
		if (i < slope_length) {
			ratio = (float)(i+1)/(float)(slope_length+1);
			p.gain  = (1.0 - ratio) * last_parameters.gain  + ratio * new_parameters.gain;
			p.peak  = (1.0 - ratio) * last_parameters.peak  + ratio * new_parameters.peak;
			p.depth = (1.0 - ratio) * last_parameters.depth + ratio * new_parameters.depth;
			p.drive = (1.0 - ratio) * last_parameters.drive + ratio * new_parameters.drive;
			
			p.threshold = p.peak - p.depth;
		} else if (i == slope_length) {
			p = new_parameters;
			p.threshold = p.peak - p.depth;
		}


		input = input[i] * p.gain;
		sign = (input < 0.0 ? -1.0 : 1.0);
		t = fabsf(input);
		
		// detector
		if (t <= p.threshold) {
			CV = 1.0f;
		} else {
			// CV
			
			// remove the threshold
			t -= p.threshold;

			// scale to the area
			t /= p.depth;

			// apply drive
			t *= p.drive;

			// tan it
			t  = tanh(t);

			// create target sample
			t = (p.threshold + t*p.depth) * sign; // scale it back
			
			// targetCV = the gain required to apply to input to reach t
			CV = t / input;
		}

		buffer_write(output_buffer[i], input[i] * p.gain * CV);

	}

	plugin_data->last_parameters = new_parameters;



    ]]></callback>



    <port label="input_gain_db" dir="input" type="control" hint="default_0">
      <name>Input gain (dB)</name>
      <p>Input gain</p>
      <range min="-20.0" max="+20.0"/>
    </port>

    <port label="max_peak_db" dir="input" type="control" hint="default_maximum">
      <name>Max level (dB)</name>
      <p>Maximum peak level to limit</p>
      <range min="-60.0" max="+0.0"/>
    </port>

    <port label="square" dir="input" type="control" hint="default_middle">
      <name>Square-waveness</name>
      <p>Saturation depth, the larger the value the more "tighter" distortion and the more square waves you get</p>
      <range min="0.0" max="+1.0"/>
    </port>

    <port label="drive" dir="input" type="control" hint="default_minimum">
      <name>Drive</name>
      <p>Drive</p>
      <range min="1.0" max="6.0"/>
    </port>


    <port label="input_buffer" dir="input" type="audio">
      <name>Input</name>
    </port>

    <port label="output_buffer" dir="output" type="audio">
      <name>Output</name>
    </port>

    <port label="latency" dir="output" type="control">
      <name>latency</name>
    </port>

    <instance-data label="last_parameters" type="SaturatorParameters"/>
    <instance-data label="samplerate" type="float"/>

  </plugin>
</ladspa>
*/

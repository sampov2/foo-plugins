/*
    Foo Lookahead Limiter - Lookahead limiter to keep peaks below a set threshold
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
		//workbuffer_left  = (float *) malloc(sizeof(float) * workbuffer_size);
		//workbuffer_right = (float *) malloc(sizeof(float) * workbuffer_size);

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
		//*(plugin_data->latency)     = ramp_up_time_samples;

		//plugin_data->current_gain   = current_gain;
		//plugin_data->envelope       = envelope;


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

      #define FOO_LIMITER_RAMP_UP_MILLISECONDS 2.5f

      #define FOO_LIMITER_MAX_LOGSCALE 10.0f

      typedef struct _Envelope {
          int ramp_samples;
	  //int sustain_samples;
          int release_samples;
          int length_samples; // ramp_samples + sustain_samples + release_samples
        
          float start_gain;
          float limit_gain;
        
          // ramp_delta = (env->limit_gain - env->start_gain)/(float)env->ramp_samples
          float ramp_delta;
        
          // release_delta = (1.0f - env->limit_gain)/(float)env->release_samples
          float release_delta;
        
          int at_sample; 


	  // Logarithmic release envelope
	  float logscale;
      } Envelope;


      // logscale parameter is given in 0.0f .. 1.0f, NOT in real scale values, this function sets the right scale
      void inline FooLimiter_triggerEnvelope(Envelope *env, int ramp_samples, int release_samples, float current_gain, float new_limit_gain, float logscale)
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
#ifdef FOO_TESTER
	  printf("envelope logscale = %f (parameter %f)\n",env->logscale, logscale);

#endif 

          env->at_sample       = 0;
      }

#define LOGSCALE(position, scale)	( logf( (position) * exp(scale) + 1.0f - (position)) / (scale))

      float FooLimiter_calculateEnvelope(Envelope *env, int offset)
      {
          int at = env->at_sample + offset;
        
          if (at >= env->length_samples) return 1.0f;
          if (at <  0) return env->start_gain;
        
          // RAMP
          if (at <  env->ramp_samples) {
              return env->start_gain + (float)at * env->ramp_delta;
          }
        
          // RELEASE

	  return env->limit_gain + ( (1.0f - env->limit_gain) * LOGSCALE( ((float) (at - env->ramp_samples))/(float)(env->release_samples), env->logscale));

          //return env->limit_gain + (float)(at - env->ramp_samples) * env->release_delta;
      }


    ]]></code>
  </global>

  <plugin label="foo_limiter" id="3181" class="LimiterPlugin">
    <name>Foo Lookahead Limiter</name>


    <callback event="instantiate"><![CDATA[
	samplerate = s_rate;
	ramp_up_time_samples = (int)floor((float)samplerate * (float)FOO_LIMITER_RAMP_UP_MILLISECONDS / 1000.0f);
#ifdef FOO_TESTER
	printf("Instantiating FooLimiter, ramp_up_time_samples = %d\n",ramp_up_time_samples);
#endif

	workbuffer_size  = 4096 + ramp_up_time_samples;
	workbuffer_left  = (float *) malloc(sizeof(float) * workbuffer_size);
	workbuffer_right = (float *) malloc(sizeof(float) * workbuffer_size);

	memset(workbuffer_left,  0, sizeof(float) * workbuffer_size);
	memset(workbuffer_right, 0, sizeof(float) * workbuffer_size);

	current_gain = 1.0f;

	// create dummy envelope which has already been passed
	// make sure triggerEnvelope doesn't use uninitialized values
	envelope.at_sample       = -1;
	envelope.ramp_samples    = 1;
	envelope.ramp_delta      = 0.0f;
	envelope.release_samples = 1;
	FooLimiter_triggerEnvelope(&envelope, 1, 1, 1.0f, 1.0f, 1.0f);
	envelope.at_sample       = 3;

    ]]></callback>

    <callback event="cleanup"><![CDATA[
	free(plugin_data->workbuffer_left);
	free(plugin_data->workbuffer_right);
    ]]></callback>

    <callback event="run"><![CDATA[

        unsigned long n,i,lookahead, buffer_offset;
	float min_gain = 1.0f;

	float peak_left, peak_right, tmp, max_peak, input_gain;

	max_peak   = DB_CO(max_peak_db);
	input_gain = DB_CO(input_gain_db);

	buffer_offset = 0;

	while (sample_count > 0) {
	    n = sample_count;

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

        	current_gain = FooLimiter_calculateEnvelope(&envelope, 0);

                // Peak detection
		peak_left  = fabsf(workbuffer_left [lookahead]);
		peak_right = fabsf(workbuffer_right[lookahead]);

		tmp = fmaxf(peak_left, peak_right) * FooLimiter_calculateEnvelope(&envelope, ramp_up_time_samples);

		if (tmp > max_peak) {

		    // calculate needed_limit_gain
		    float needed_limit_gain = max_peak / fmaxf(peak_left, peak_right);

		    int release_samples = (int)floor( release_time * (float)samplerate);

#ifdef FOO_TESTER			    
		    printf("new envelope! release samples = %d, max_peak = %f, current_gain = %f, peak = %f, limit_gain = %f\n", release_samples, max_peak, current_gain, tmp, needed_limit_gain);
#endif
                    FooLimiter_triggerEnvelope(&envelope, ramp_up_time_samples, release_samples, current_gain, needed_limit_gain, release_scale);
	        }
                
		if (current_gain < min_gain) min_gain = current_gain;

		if (envelope.at_sample <= envelope.length_samples)
		    envelope.at_sample++;


		buffer_write(output_left [i+buffer_offset], workbuffer_left [i] * current_gain);
#ifndef FOO_TESTER
		buffer_write(output_right[i+buffer_offset], workbuffer_right[i] * current_gain);
#else
		buffer_write(output_right[i+buffer_offset], current_gain);
		if (fabsf(output_left[i+buffer_offset]) > max_peak) {
			printf("######### Limiter failed at %ld, max_peak = %f and peaked at %f\n", i+buffer_offset, max_peak, output_left[i+buffer_offset]);
			printf("## Envelope: @%d, ramp_delta = %f, release_delta = %f, ramp_samples = %d, release_samples = %d, start_gain = %f, limit_gain = %f\n",
			       envelope.at_sample,
			       envelope.ramp_delta,
			       envelope.release_delta,
			       envelope.ramp_samples,
			       envelope.release_samples,
			       envelope.start_gain,
			       envelope.limit_gain);
		}
#endif

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

	    sample_count -= n;
	    buffer_offset += n;
	}

	*(plugin_data->attenuation) = -CO_DB(min_gain);
	*(plugin_data->latency)     = ramp_up_time_samples;

	plugin_data->current_gain   = current_gain;
	plugin_data->envelope       = envelope;

    ]]></callback>



    <port label="input_gain_db" dir="input" type="control" hint="default_0">
      <name>Input gain (dB)</name>
      <p>Maximum peak level to limit</p>
      <range min="-20.0" max="+10.0"/>
    </port>

    <port label="max_peak_db" dir="input" type="control" hint="default_0">
      <name>Max level (dB)</name>
      <p>Maximum peak level to limit</p>
      <range min="-30.0" max="+0.0"/>
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

    <port label="latency" dir="output" type="control">
      <name>latency</name>
    </port>


    <port label="release_scale" dir="input" type="control" hint="default_high">
      <name>Linear/log release</name>
      <p>Limiter release time in milliseconds</p>
      <range min="0.0" max="1.0"/>
    </port>


    <instance-data label="ramp_up_time_samples" type="int"/>
    <instance-data label="samplerate" type="unsigned long"/>
    <instance-data label="current_gain" type="float"/>
    <instance-data label="envelope" type="Envelope"/>

    <instance-data label="workbuffer_size"  type="int"/>
    <instance-data label="workbuffer_left"  type="float *"/>
    <instance-data label="workbuffer_right" type="float *"/>


  </plugin>
</ladspa>
*/

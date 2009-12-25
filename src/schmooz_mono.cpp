/*
    Smooth sounding compressor with built-in support for parallel compression
    Copyright (C) 2009  Sampo Savolainen <v2@iki.fi>

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

//#include <iostream>

inline float max(float a, float b) { return fmax(a, b); }
inline float min(float a, float b) { return fmin(a, b); }

#include "generated/faust-minimal-schmooz-mono.cpp"

using namespace LV2;
using namespace std;

// These need to match the indexes in manifest.ttl
#define PORT_AUDIO_INPUT        0
#define PORT_AUDIO_OUTPUT       1
#define PORT_THRESHOLD          2
#define PORT_SIDECHAIN_HPF      3
#define PORT_ATTACK             4
#define PORT_RELEASE            5
#define PORT_RATIO              6
#define PORT_MAKEUP             7
#define PORT_DRYWET             8

#define PORT_OUTPUT_ATTENUATION 9


class SchmoozMono : public Plugin<SchmoozMono>, UI {
public:
  SchmoozMono(double rate)
        : Plugin<SchmoozMono>(10)
	, max_nframes(32768)
  {
	schmooz_mono.instanceInit( (int)rate );

	schmooz_mono.buildUserInterface(this);

	input_measurement_buffer       = new float[max_nframes];
	attenuation_measurement_buffer = new float[max_nframes];
  }

  void run(uint32_t nframes) 
  {
	*threshold_db 		= *p(PORT_THRESHOLD);
	*sidechain_hpf_enabled 	= (*p(PORT_SIDECHAIN_HPF) > 0 ? 1 : 0);
	*attack_ms 		= *p(PORT_ATTACK);
	*release_ms 		= *p(PORT_RELEASE);
	*compression_ratio 	= *p(PORT_RATIO);
	*makeup_gain_db 	= *p(PORT_MAKEUP);
	*dry_wet_balance 	= *p(PORT_DRYWET);

	uint32_t n = 0;
	float *outputBuffers[3];

	float input_peak = -70.0;
	float compressed_peak = 200;
	float full_atten_gain = 0;
	float full_atten_gain_abs = 0;

	while ( n < nframes ) {

		outputBuffers[0] = p(PORT_AUDIO_OUTPUT) + n;
		outputBuffers[1] = attenuation_measurement_buffer + n;
		outputBuffers[2] = input_measurement_buffer + n;

		uint32_t i = nframes - n;
		if (i > max_nframes) {
			i = max_nframes;
		}
		//std::cerr << " i = " << i << std::endl;

		schmooz_mono.compute(i, &p(PORT_AUDIO_INPUT) + n, outputBuffers);

		n += i;

		while ( i > 0) {
			--i;
			// current input (after HPF and RMS follower)
			float curr_input = CO_DB(input_measurement_buffer[i]);

			float compgain   = attenuation_measurement_buffer[i]; // in dB

			// current raw compressed input is input adjusted 
			// by compressor gain with makeup negated
			float curr_comp  = curr_input + compgain - *makeup_gain_db;

			// full attenuation (gain) is compgain adjusted by dry/wet
			float full = compgain * *dry_wet_balance;
			float full_abs = fabsf(full);

			// is max right, or should we use some sort of smoothing?
			input_peak       = fmaxf(input_peak, curr_input);
			compressed_peak  = fmaxf(compressed_peak, curr_comp);

			// 
			if (full_atten_gain_abs < full_abs) {
				full_atten_gain = full;
				full_atten_gain_abs = full_abs;
			}
		}

	}
	//std::cerr << "--------" << std::endl;
	//std::cerr << "input_peak = " << input_peak << std::endl;
	//std::cerr << "compressed_peak = " << compressed_peak << std::endl;
	//std::cerr << "full_atten_gain = " << full_atten_gain << std::endl;

	*p(PORT_OUTPUT_ATTENUATION) = full_atten_gain;
  }

  // Unused UI-functions        
  void addButton(const char* label, float* zone) {}
  void addToggleButton(const char* label, float* zone) {}
  void addCheckButton(const char* label, float* zone) {}
  void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step) {}
  void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) {}
  void openFrameBox(const char* label) {}
  void openTabBox(const char* label) {}
  void openHorizontalBox(const char* label) {}
  void openVerticalBox(const char* label) {}
  void closeBox() {}
  void declare(float* zone, const char* key, const char* value) {};


  void addHorizontalSlider(const char* l, float* zone, float init, float min, float max, float step) {
	string label(l);
	//cerr << "schmooz, addHorizontalSlider() with label '" << label << "'" << endl;
	
	if (label == "attack (ms)") {
		attack_ms = zone;

	} else if (label == "compression ratio") {
		compression_ratio = zone;

	} else if (label == "dry-wet") {
		dry_wet_balance = zone;

	} else if (label == "makeup gain (dB)") {
		makeup_gain_db = zone;
		
	} else if (label == "release (ms)") {
		release_ms = zone;

	} else if (label == "sidechain hpf") {
        	sidechain_hpf_enabled = zone;

	} else if (label == "threshold (dB)") {
		threshold_db = zone;

	//} else {
	//	cerr << "schmooz, addHorizontalSlider() with unknown label " << label << endl;
	}
  }
        

  private:
        float   *attack_ms;
        float   *compression_ratio;
        float   *dry_wet_balance;
        float   *makeup_gain_db;
        float   *release_ms;
        float   *sidechain_hpf_enabled;
        float   *threshold_db;

	mydsp schmooz_mono;

	uint32_t max_nframes;
	float   *input_measurement_buffer;
	float   *attenuation_measurement_buffer;
};

static int _ = SchmoozMono::register_class("http://studionumbersix.com/foo/lv2/schmooz-mono");


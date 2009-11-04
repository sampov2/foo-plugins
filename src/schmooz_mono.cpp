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

inline float max(float a, float b) { return fmax(a, b); }
inline float min(float a, float b) { return fmin(a, b); }

#include "generated/faust-minimal-schmooz-mono.cpp"

using namespace LV2;
using namespace std;

// These need to match the indexes in manifest.ttl
#define PORT_AUDIO_INPUT        0
#define PORT_AUDIO_OUTPUT       1
#define PORT_THRESHOLD          2
#define PORT_SIDECHAIN          3
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
  {
	schmooz_mono.instanceInit( (int)rate );

	schmooz_mono.buildUserInterface(this);
  }

  void run(uint32_t nframes) 
  {
	double input_avg  = 0.0;
	double output_avg = 0.0;

	*threshold_db 		= *p(PORT_THRESHOLD);
	*sidechain_enabled 	= (*p(PORT_SIDECHAIN) > 0 ? 1 : 0);
	*attack_ms 		= *p(PORT_ATTACK);
	*release_ms 		= *p(PORT_RELEASE);
	*compression_ratio 	= *p(PORT_RATIO);
	*makeup_gain_db 	= *p(PORT_MAKEUP);
	*dry_wet_balance 	= *p(PORT_DRYWET);

	for (uint32_t x = 0; x < nframes; ++x) {
		input_avg  += p(PORT_AUDIO_INPUT)[x];
	}

	schmooz_mono.compute(nframes, &p(PORT_AUDIO_INPUT), &p(PORT_AUDIO_OUTPUT));

	for (uint32_t x = 0; x < nframes; ++x) {
		output_avg += p(PORT_AUDIO_OUTPUT)[x];
	}

	if (input_avg == 0.0 || output_avg == 0.0) {
		*p(PORT_OUTPUT_ATTENUATION) = 0.0;
	} else {
		*p(PORT_OUTPUT_ATTENUATION) = log10f(output_avg / input_avg) * 20.0;
	}
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
        	sidechain_enabled = zone;

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
        float   *sidechain_enabled;
        float   *threshold_db;

	mydsp schmooz_mono;

};

static int _ = SchmoozMono::register_class("http://studionumbersix.com/foo/lv2/schmooz-mono");


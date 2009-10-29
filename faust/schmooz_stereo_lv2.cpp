#include <lv2plugin.hpp>
//#include <iostream>
#include <cmath>

inline float max(float a, float b) { return fmax(a, b); }
inline float min(float a, float b) { return fmin(a, b); }

#include "gen/foo-comp-stereo-minimal.cpp"

using namespace LV2;
using namespace std;

// These need to match the indexes in manifest.ttl
#define PORT_AUDIO_INPUT_L      0
#define PORT_AUDIO_OUTPUT_L     1
#define PORT_THRESHOLD          2
#define PORT_SIDECHAIN          3
#define PORT_ATTACK             4
#define PORT_RELEASE            5
#define PORT_RATIO              6
#define PORT_MAKEUP             7
#define PORT_DRYWET             8

#define PORT_OUTPUT_ATTENUATION 9
#define PORT_AUDIO_INPUT_R      10
#define PORT_AUDIO_OUTPUT_R     11


class SchmoozStereo : public Plugin<SchmoozStereo>, UI {
public:
    SchmoozStereo(double rate)
          : Plugin<SchmoozStereo>(12)
  {
	schmooz_stereo.instanceInit( (int)rate );

	schmooz_stereo.buildUserInterface(this);
  }

  void run(uint32_t nframes) 
  {
	double input_avg  = 0.0;
	double output_avg = 0.0;

	*threshold_db 		= *p(PORT_THRESHOLD);
	*sidechain_enabled 	= *p(PORT_SIDECHAIN);
	*attack_ms 		= *p(PORT_ATTACK);
	*release_ms 		= *p(PORT_RELEASE);
	*compression_ratio 	= *p(PORT_RATIO);
	*makeup_gain_db 	= *p(PORT_MAKEUP);
	*dry_wet_balance 	= *p(PORT_DRYWET);

	for (uint32_t x = 0; x < nframes; ++x) {
		input_avg  += p(PORT_AUDIO_INPUT_L)[x];
	}

	float *in[2], *out[2];

	in[0]  = p(PORT_AUDIO_INPUT_L);
	in[1]  = p(PORT_AUDIO_INPUT_R);
	out[0] = p(PORT_AUDIO_OUTPUT_L);
	out[1] = p(PORT_AUDIO_OUTPUT_R);
	

	schmooz_stereo.compute(nframes, in, out);

	for (uint32_t x = 0; x < nframes; ++x) {
		output_avg += p(PORT_AUDIO_OUTPUT_L)[x];
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
        float *attack_ms;
        float *compression_ratio;
        float *dry_wet_balance;
        float *makeup_gain_db;
        float *release_ms;
        float *sidechain_enabled;
        float *threshold_db;

	mydsp schmooz_stereo;

};

static int _ = SchmoozStereo::register_class("http://studionumbersix.com/foo/lv2/schmooz/stereo-comp");


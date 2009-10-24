#include <lv2plugin.hpp>
#include <cmath>


using namespace LV2;
using namespace std;

// These need to match the indexes in manifest.ttl
#define PORT_AUDIO_INPUT	0
#define PORT_AUDIO_OUTPUT	1
#define PORT_THRESHOLD		2
#define PORT_SIDECHAIN		3
#define PORT_ATTACK		4
#define PORT_RELEASE		5
#define PORT_RATIO		6
#define PORT_MAKEUP		7
#define PORT_DRYWET		8

#define PORT_OUTPUT_ATTENUATION 9

class Schmooz : public Plugin<Schmooz> {
public:
  
  Schmooz(double rate)
	  : Plugin<Schmooz>(10) 
  {
	  fSamplingFreq = rate;
	  attack_ms = 10.0f;
	  fConst0 = (339.785217f / fSamplingFreq);
	  release_ms = 300.0f;
	  fConst1 = (679.570435f / fSamplingFreq);
	  threshold_db = -30.0f;
	  fConst2 = (31.415928f / fSamplingFreq);
	  fConst3 = sinf(fConst2);
	  fConst4 = (fConst3 * sinhf((32.663792f / (fSamplingFreq * fConst3))));
	  fConst5 = (fConst4 - 1.0f);
	  fConst6 = cosf(fConst2);
	  fConst7 = (2.0f * fConst6);
	  for (int i=0; i<3; i++) fVec0[i] = 0;
	  fConst8 = (1.0f + fConst6);
	  fConst9 = (0.5f * fConst8);
	  fConst10 = (0 - fConst8);
	  fConst11 = (1.0f / (1.0f + fConst4));
	  for (int i=0; i<3; i++) fRec3[i] = 0;
	  S5[0] = 1.0f;
	  S5[1] = 0.0f;
	  sidechain_enabled = 0.0f;
	  fConst12 = (471.238922f / fSamplingFreq);
	  fConst13 = sinf(fConst12);
	  fConst14 = (fConst13 * sinhf((489.956909f / (fSamplingFreq * fConst13))));
	  fConst15 = (fConst14 - 1.0f);
	  fConst16 = cosf(fConst12);
	  fConst17 = (2.0f * fConst16);
	  fConst18 = (1.0f + fConst16);
	  fConst19 = (0.5f * fConst18);
	  fConst20 = (0 - fConst18);
	  fConst21 = (1.0f / (1.0f + fConst14));
	  for (int i=0; i<3; i++) fRec4[i] = 0;
	  IOTA = 0;
	  for (int i=0; i<128; i++) iVec1[i] = 0;
	  fConst22 = min(192000.0f, max(22050.0f, fSamplingFreq));
	  iConst23 = int((5.000000e-04f * fConst22));
	  for (int i=0; i<2; i++) iRec2[i] = 0;
	  fConst24 = (1.907349e-03f / fConst22);
	  for (int i=0; i<2; i++) fRec1[i] = 0;
	  compression_ratio = 5.0f;
	  makeup_gain_db = 0.0f;
	  S6[0] = 1.0f;
	  S6[1] = -1.0f;
	  fConst25 = (96.0f / fSamplingFreq);
	  for (int i=0; i<3; i++) fRec0[i] = 0;
	  S0[1] = 0;
	  dry_wet_balance = 1.0f;
  }
  
  void run(uint32_t nframes) 
  {
	  float max_attenuation = 1.0;

	  threshold_db		= *p(PORT_THRESHOLD);
	  sidechain_enabled	= *p(PORT_SIDECHAIN);
	  attack_ms		= *p(PORT_ATTACK);
	  release_ms		= *p(PORT_RELEASE);
	  compression_ratio	= *p(PORT_RATIO);
	  makeup_gain_db	= *p(PORT_MAKEUP);
	  dry_wet_balance	= *p(PORT_DRYWET);

	

	  float 	S4[2];
	  float 	fSlow0 = (fConst0 / attack_ms);
	  float 	fSlow1 = (fConst1 / release_ms);
	  float 	fSlow2 = threshold_db;
	  float 	fSlow3 = S5[(sidechain_enabled > 0 ? 1 : 0)];
	  float 	fSlow4 = (1.0f - fSlow3);
	  float 	fSlow5 = (1 - (1.0f / compression_ratio));
	  float 	fSlow6 = makeup_gain_db;
	  float 	fSlow7 = dry_wet_balance;
	  float 	fSlow8 = (1 - fSlow7);
	  S4[0] = fSlow0;
	  S4[1] = fSlow1;
	  float* input0  = p(PORT_AUDIO_INPUT);
	  float* output0 = p(PORT_AUDIO_OUTPUT);
	  for (uint32_t i=0; i < nframes; i++) {
		  float 	S1[2];
		  float 	S2[2];
		  float 	S3[2];
		  float fTemp0 = input0[i];
		  fVec0[0] = fTemp0;
		  fRec3[0] = (fConst11 * (((((fConst10 * fVec0[1]) + (fConst9 * fVec0[0])) + (fConst9 * fVec0[2])) + (fConst7 * fRec3[1])) + (fConst5 * fRec3[2])));
		  fRec4[0] = (fConst21 * (((((fConst20 * fVec0[1]) + (fConst19 * fVec0[0])) + (fConst19 * fVec0[2])) + (fConst17 * fRec4[1])) + (fConst15 * fRec4[2])));
		  float fTemp1 = ((fSlow4 * fRec4[0]) + (fSlow3 * fRec3[0]));
		  int iTemp2 = int((1048576 * (fTemp1 * fTemp1)));
		  iVec1[IOTA&127] = iTemp2;
		  iRec2[0] = ((iVec1[IOTA&127] + iRec2[1]) - iVec1[(IOTA-iConst23)&127]);
		  float fTemp3 = (20 * log10f(sqrtf((fConst24 * float(iRec2[0])))));
		  float fTemp4 = ((fSlow2 < fTemp3) * (fTemp3 - fSlow2));
		  float fTemp5 = S4[int((fTemp4 < fRec1[1]))];
		  fRec1[0] = ((fRec1[1] * (1 - fTemp5)) + (fTemp4 * fTemp5));
		  float fTemp6 = (fSlow5 * fRec1[0]);
		  float fTemp7 = (fSlow6 - fTemp6);
		  S3[0] = fTemp7;
		  float fTemp8 = (2 * fRec0[1]);
		  S3[1] = (((fConst25 * S6[int((fTemp7 < fRec0[1]))]) + fTemp8) - fRec0[2]);
		  float fTemp9 = S3[int((fabsf((0 - ((fSlow6 + fRec0[2]) - (fTemp6 + fTemp8)))) > fConst25))];
		  S2[0] = fTemp9;
		  S2[1] = fTemp7;
		  int iTemp10 = int((fTemp9 > fTemp7));
		  S1[0] = S2[iTemp10];
		  float 	S7[2];
		  S7[0] = fTemp7;
		  S7[1] = fTemp9;
		  S1[1] = S7[iTemp10];
		  fRec0[0] = S1[int((((fRec0[1] + fTemp6) - fSlow6) > 0.0f))];
		  S0[0] = powf(10, (5.000000e-02f * fRec0[0]));

		  float gain = (fSlow8 + (fSlow7 * S0[int((fRec0[0] < -318.799988f))]));
		  //output0[i] = (fVec0[0] * (fSlow8 + (fSlow7 * S0[int((fRec0[0] < -318.799988f))])));
		  output0[i] = (fVec0[0] * gain);

		  if (i == 0 || gain < max_attenuation) {
			max_attenuation = gain;
		  }
		  // post processing
		  fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
		  fRec1[1] = fRec1[0];
		  iRec2[1] = iRec2[0];
		  IOTA = IOTA+1;
		  fRec4[2] = fRec4[1]; fRec4[1] = fRec4[0];
		  fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
		  fVec0[2] = fVec0[1]; fVec0[1] = fVec0[0];
	  }
	  max_attenuation = log10f(max_attenuation) * 20.0f;
	  if (isnan(max_attenuation)) {
	  	max_attenuation = 0.0;
	  }
	  *p(PORT_OUTPUT_ATTENUATION) = max_attenuation;
  }

  private:
	float 	threshold_db;
	float 	sidechain_enabled;
	float 	attack_ms;
	float 	release_ms;
	float 	compression_ratio;
	float 	makeup_gain_db;
	float 	dry_wet_balance;

	float 	S0[2];
	float 	fConst0;
	float 	fConst1;
	float 	fConst2;
	float 	fConst3;
	float 	fConst4;
	float 	fConst5;
	float 	fConst6;
	float 	fConst7;
	float 	fVec0[3];
	float 	fConst8;
	float 	fConst9;
	float 	fConst10;
	float 	fConst11;
	float 	fRec3[3];
	float 	S5[2];
	float 	fConst12;
	float 	fConst13;
	float 	fConst14;
	float 	fConst15;
	float 	fConst16;
	float 	fConst17;
	float 	fConst18;
	float 	fConst19;
	float 	fConst20;
	float 	fConst21;
	float 	fRec4[3];
	int 	IOTA;
	int 	iVec1[128];
	float 	fConst22;
	int 	iConst23;
	int 	iRec2[2];
	float 	fConst24;
	float 	fRec1[2];
	float 	S6[2];
	float 	fConst25;
	float 	fRec0[3];

	float fSamplingFreq;

};


static int _ = Schmooz::register_class("http://studionumbersix.com/foo/lv2/schmooz-comp");


//-----------------------------------------------------
// name: "foo schmoozcomp mono"
// author: "Sampo Savolainen"
// version: "0.9b"
// copyright: "(c)Sampo Savolainen 2009"
//
// Code generated with Faust 0.9.10 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with : "" */
#include <math.h>
#include <cmath>
// abs is now predefined
//template<typename T> T abs (T a)			{ return (a<T(0)) ? -a : a; }


inline int		lsr (int x, int n)			{ return int(((unsigned int)x) >> n); }

/******************************************************************************
*******************************************************************************

							       VECTOR INTRINSICS

*******************************************************************************
*******************************************************************************/

//inline void *aligned_calloc(size_t nmemb, size_t size) { return (void*)((unsigned)(calloc((nmemb*size)+15,sizeof(char)))+15 & 0xfffffff0); }
//inline void *aligned_calloc(size_t nmemb, size_t size) { return (void*)((size_t)(calloc((nmemb*size)+15,sizeof(char)))+15 & ~15); }


/******************************************************************************
*******************************************************************************

			ABSTRACT USER INTERFACE

*******************************************************************************
*******************************************************************************/

struct Meta 
{
    void declare (const char*, const char*) {  }
};

//----------------------------------------------------------------
//  abstract definition of a user interface
//----------------------------------------------------------------
            

class UI
{
	bool	fStopped;
public:
		
	UI() : fStopped(false) {}
	virtual ~UI() {}
	
	virtual void addButton(const char* label, float* zone) = 0;
	virtual void addToggleButton(const char* label, float* zone) = 0;
	virtual void addCheckButton(const char* label, float* zone) = 0;
	virtual void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step) = 0;
	virtual void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step) = 0;
	virtual void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) = 0;
	
	virtual void openFrameBox(const char* label) = 0;
	virtual void openTabBox(const char* label) = 0;
	virtual void openHorizontalBox(const char* label) = 0;
	virtual void openVerticalBox(const char* label) = 0;
	virtual void closeBox() = 0;

    virtual void declare(float* zone, const char* key, const char* value) = 0;
};




/******************************************************************************
*******************************************************************************

			    FAUST DSP

*******************************************************************************
*******************************************************************************/



//----------------------------------------------------------------
//  abstract definition of a signal processor
//----------------------------------------------------------------
			
class dsp {
 protected:
	int fSamplingFreq;
 public:
	dsp() {}
	virtual ~dsp() {}

	virtual int getNumInputs() 						= 0;
	virtual int getNumOutputs() 					= 0;
	virtual void buildUserInterface(UI* interface) 	= 0;
	virtual void init(int samplingRate) 			= 0;
 	virtual void compute(int len, float** inputs, float** outputs) 	= 0;
};
		

//----------------------------------------------------------------------------
//  FAUST generated signal processor
//----------------------------------------------------------------------------
		

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;

class mydsp : public dsp{
  private:
	float 	S0[2];
	FAUSTFLOAT 	fslider0;
	float 	fConst0;
	FAUSTFLOAT 	fslider1;
	float 	fConst1;
	FAUSTFLOAT 	fslider2;
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
	FAUSTFLOAT 	fslider3;
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
	FAUSTFLOAT 	fslider4;
	FAUSTFLOAT 	fslider5;
	float 	S6[2];
	float 	fConst25;
	float 	fRec0[3];
	FAUSTFLOAT 	fslider6;
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "foo schmoozcomp mono");
		m->declare("author", "Sampo Savolainen");
		m->declare("version", "0.9b");
		m->declare("copyright", "(c)Sampo Savolainen 2009");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 3; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 10.0f;
		fConst0 = (339.785229f / fSamplingFreq);
		fslider1 = 300.0f;
		fConst1 = (1812.187886f / fSamplingFreq);
		fslider2 = -10.0f;
		fConst2 = (31.415927f / fSamplingFreq);
		fConst3 = sinf(fConst2);
		fConst4 = (fConst3 * sinhf((32.663791f / (fSamplingFreq * fConst3))));
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
		fslider3 = 0.0f;
		fConst12 = (471.238898f / fSamplingFreq);
		fConst13 = sinf(fConst12);
		fConst14 = (fConst13 * sinhf((489.95687f / (fSamplingFreq * fConst13))));
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
		fslider4 = 5.0f;
		fslider5 = 0.0f;
		S6[0] = 1.0f;
		S6[1] = -1.0f;
		fConst25 = (96.0f / fSamplingFreq);
		for (int i=0; i<3; i++) fRec0[i] = 0;
		S0[1] = 0;
		fslider6 = 1.0f;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("foo-comp-mono");
		interface->addHorizontalSlider("attack (ms)", &fslider0, 10.0f, 0.1f, 120.0f, 0.1f);
		interface->addHorizontalSlider("compression ratio", &fslider4, 5.0f, 1.5f, 20.0f, 0.5f);
		interface->addHorizontalSlider("dry-wet", &fslider6, 1.0f, 0.0f, 1.0f, 0.1f);
		interface->addHorizontalSlider("makeup gain (dB)", &fslider5, 0.0f, 0.0f, 40.0f, 0.5f);
		interface->addHorizontalSlider("release (ms)", &fslider1, 300.0f, 50.0f, 1200.0f, 1.0f);
		interface->addHorizontalSlider("sidechain hpf", &fslider3, 0.0f, 0.0f, 1.0f, 1.0f);
		interface->addHorizontalSlider("threshold (dB)", &fslider2, -10.0f, -60.0f, 10.0f, 1.0f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	S4[2];
		float 	fSlow0 = (fConst0 / fslider0);
		float 	fSlow1 = (fConst1 / fslider1);
		float 	fSlow2 = fslider2;
		float 	fSlow3 = S5[int(fslider3)];
		float 	fSlow4 = (1.0f - fSlow3);
		float 	fSlow5 = (1 - (1.0f / fslider4));
		float 	fSlow6 = fslider5;
		float 	fSlow7 = fslider6;
		float 	fSlow8 = (1 - fSlow7);
		S4[0] = fSlow0;
		S4[1] = fSlow1;
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		FAUSTFLOAT* output2 = output[2];
		for (int i=0; i<count; i++) {
			float 	S1[2];
			float 	S2[2];
			float 	S3[2];
			float fTemp0 = (float)input0[i];
			fVec0[0] = fTemp0;
			fRec3[0] = (fConst11 * (((((fConst10 * fVec0[1]) + (fConst9 * fVec0[0])) + (fConst9 * fVec0[2])) + (fConst7 * fRec3[1])) + (fConst5 * fRec3[2])));
			fRec4[0] = (fConst21 * (((((fConst20 * fVec0[1]) + (fConst19 * fVec0[0])) + (fConst19 * fVec0[2])) + (fConst17 * fRec4[1])) + (fConst15 * fRec4[2])));
			float fTemp1 = ((fSlow4 * fRec4[0]) + (fSlow3 * fRec3[0]));
			int iTemp2 = int((1048576 * (fTemp1 * fTemp1)));
			iVec1[IOTA&127] = iTemp2;
			iRec2[0] = ((iVec1[IOTA&127] + iRec2[1]) - iVec1[(IOTA-iConst23)&127]);
			float fTemp3 = sqrtf((fConst24 * float(iRec2[0])));
			float fTemp4 = (20 * log10f(fTemp3));
			float fTemp5 = ((fSlow2 < fTemp4) * (fTemp4 - fSlow2));
			float fTemp6 = S4[int((fTemp5 < fRec1[1]))];
			fRec1[0] = ((fRec1[1] * (1 - fTemp6)) + (fTemp5 * fTemp6));
			float fTemp7 = (fSlow5 * fRec1[0]);
			float fTemp8 = (fSlow6 - fTemp7);
			S3[0] = fTemp8;
			float fTemp9 = (2 * fRec0[1]);
			S3[1] = (((fConst25 * S6[int((fTemp8 < fRec0[1]))]) + fTemp9) - fRec0[2]);
			float fTemp10 = S3[int((fabsf((0 - ((fSlow6 + fRec0[2]) - (fTemp7 + fTemp9)))) > fConst25))];
			S2[0] = fTemp10;
			S2[1] = fTemp8;
			int iTemp11 = int((fTemp10 > fTemp8));
			S1[0] = S2[iTemp11];
			float 	S7[2];
			S7[0] = fTemp8;
			S7[1] = fTemp10;
			S1[1] = S7[iTemp11];
			fRec0[0] = S1[int((((fRec0[1] + fTemp7) - fSlow6) > 0.0f))];
			S0[0] = powf(10,(5.000000e-02f * fRec0[0]));
			output0[i] = (FAUSTFLOAT)(fVec0[0] * (fSlow8 + (fSlow7 * S0[int((fRec0[0] < -318.8f))])));
			output1[i] = (FAUSTFLOAT)fRec0[0];
			output2[i] = (FAUSTFLOAT)fTemp3;
			// post processing
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			fRec1[1] = fRec1[0];
			iRec2[1] = iRec2[0];
			IOTA = IOTA+1;
			fRec4[2] = fRec4[1]; fRec4[1] = fRec4[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fVec0[2] = fVec0[1]; fVec0[1] = fVec0[0];
		}
	}
};



#ifndef RMS_H
#define RMS_H

#include <stdint.h>

namespace Foo {

class RMS
{
public:
	RMS(float _fs, float _time);

	float run(float x);
	float run_buffer(float *x, uint32_t nframes);

	void set_time(float _time);

private:
	double fs;
	double rm;
	double coef;
};

};

#endif


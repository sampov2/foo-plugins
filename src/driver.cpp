/*
    Foo Driver - Warm up your signal
    Copyright (C) 2006-2009  Sampo Savolainen <v2@iki.fi>

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

using namespace LV2;
using namespace std;

#define PORT_INPUT              0
#define PORT_OUTPUT             1
#define PORT_GAIN		2
#define PORT_DRIVE		3
#define PORT_OFFSET		4
#define PORT_BALANCE		5

#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

class Driver : public Plugin<Driver>
{
public:
	Driver(double rate)
		: Plugin<Driver>(6)
	{		
	}


	void run(uint32_t nframes)
	{
		float *input  =  p(PORT_INPUT);
		float *output =  p(PORT_OUTPUT);
		float drive   = *p(PORT_DRIVE);
		float offset  = *p(PORT_OFFSET);
		float balance = *p(PORT_BALANCE);
		float input_gain_coef = DB_CO(*p(PORT_GAIN));

		float wet;

		for (uint32_t i = 0; i < nframes; i++) {
			wet = tanh(input[i] * input_gain_coef * drive + offset) - tanh(offset);

			output[i] = wet * (1.0f - balance) + input[i] * input_gain_coef * balance;
		}
	}
};

static int _ = Driver::register_class("http://studionumbersix.com/foo/lv2/driver");


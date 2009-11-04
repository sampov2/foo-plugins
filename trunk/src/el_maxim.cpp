/*
    El Maxim - maximizer / saturator
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

using namespace LV2;
using namespace std;

#define PORT_INPUT              0
#define PORT_OUTPUT             1
#define PORT_INPUT_GAIN         2
#define PORT_AMOUNT             3
#define PORT_CLIP_INDICATOR	4

class ElMaxim : public Plugin<ElMaxim>
{
public:
	ElMaxim(double rate)
		: Plugin<ElMaxim>(5)
	{

	}

	void run(uint32_t nframes)
	{
		uint32_t clips = 0;

		float *input 	 = p(PORT_INPUT);
		float *output 	 = p(PORT_OUTPUT);
		float input_gain = DB_CO( *p(PORT_INPUT_GAIN) );
		float amount     = *p(PORT_AMOUNT);

		for (uint32_t i = 0 ; i < nframes; ++i)
		{
			float tmp = input[i] * input_gain;
			if (fabsf(tmp) > 1.0) {
				++clips;
				tmp = (tmp < 0.0 ? -1.0 : 1.0);
			}
			
			output[i] = tmp + sin(tmp * M_PI) * 0.31831 * amount;
		}


		*p(PORT_CLIP_INDICATOR) = (float)clips/(float)nframes;
	}

};

static int _ = ElMaxim::register_class("http://studionumbersix.com/foo/lv2/el-maxim");


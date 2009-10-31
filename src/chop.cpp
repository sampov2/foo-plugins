/*
    Foo Chop Liver - wave chopper distortion
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

using namespace LV2;

#define PORT_INPUT		0
#define PORT_OUTPUT		1
#define PORT_CHOP_WINDOW	2
#define PORT_DRY_GAIN		3
#define PORT_WET_GAIN		4

class ChopLiver : public Plugin<ChopLiver> 
{
public:
	ChopLiver(double rate) 
		: Plugin<ChopLiver>(5)
	{
		window = new float[32];
		
	}

	~ChopLiver() 
	{
		delete window;
	}

	void run(uint32_t nframes)
	{
		float *input  = p(PORT_INPUT);
		float *output = p(PORT_OUTPUT);
		uint32_t window_size = (uint32_t)*p(PORT_CHOP_WINDOW);
		float dry_gain = *p(PORT_DRY_GAIN);
		float wet_gain = *p(PORT_WET_GAIN);
		 

		uint32_t i;
		//printf("window_size = %d\n",window_size);

		if (window_size < 2) window_size = 2;
		if (window_size > 32) window_size = 32;

		while (nframes > 0) {
			//printf("samples left %d, window %d\n",nframes, window_size);

			for (i = 0; i < window_size; ++i) {
				window[i] = input[window_size - i - 1];
			}

			for (i = 0; i < window_size; ++i) {
				output[i] = input [i] * dry_gain +
					    window[i] * wet_gain;
			}

			output += window_size;
			input  += window_size;

			nframes -= window_size;

			// last window will be the size of the samples left over
			if (window_size > nframes)
				window_size = nframes; 
		}
	}

private:
	float *window;
};

static int _ = ChopLiver::register_class("http://studionumbersix.com/foo/lv2/chop-liver");


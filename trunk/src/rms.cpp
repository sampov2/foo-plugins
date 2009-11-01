/*
 *  Copyright (C) 2006 Steve Harris
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  $Id: rms.c $
 */

#include <math.h>
#include <stdlib.h>

#include "rms.h"

#define NEAR_ZERO 0.0001f

namespace Foo {

RMS::RMS(float _fs, float _time)
{
	fs   = _fs;
	rm   = NEAR_ZERO;
    	coef = 0.5 * (1.0 - exp(-1.0 / (_fs * _time)));
}

float
RMS::run(float x)
{
    if (rm < NEAR_ZERO) rm = NEAR_ZERO;

    rm += coef * (( (x * x) / rm) - rm);

    return rm;
}

float 
RMS::run_buffer(float *x, uint32_t nframes)
{
    for (uint32_t i=0; i<nframes; ++i) {
        if (rm < NEAR_ZERO) rm = NEAR_ZERO;
        rm += coef * ((x[i] * x[i] / rm) - rm);
    }

    return rm;
}

};

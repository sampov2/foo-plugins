/*
 *  Copyright (C) 2009 Sampo Savolainen
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
 */

declare name      "foo schmoozcomp mono";
declare author    "Sampo Savolainen";
declare version   "0.9b";
declare copyright "(c)Sampo Savolainen 2009";

import ("math.lib");

import ("compressor-basics.dsp");

import ("biquad-hpf.dsp");


metering_speed    = 0.1 * min(192000.0, max(22050.0, SR));
attenuation_speed = 0.0005 * min(192000.0, max(22050.0, SR));



DRYWET2(ratio,dry,wet) = dry * ratio + wet * (1 - ratio);

GAIN(signal, gain) = signal * gain, signal;

COMP = HPF : RMS(rms_speed) <: (DETECTOR : RATIO : ( RATELIMITER ~ _) <: DB2COEFF, (_) ), (_);

// the mean() function is defined in rms.dsp
process = _ <: _, COMP : GAIN, _, _ : DRYWET2(drywet), mean(attenuation_speed), mean(metering_speed);

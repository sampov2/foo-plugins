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



COMP = _ <: ( HPF : DETECTOR : RATIO : ( RATELIMITER ~ _ ) : DB2COEFF );

//COMP = _ <: ( DETECTOR : RATIO : DB2COEFF );

process =  _ <: ( _ , *(COMP) ) : DRYWET(drywet);

//process =  DETECTOR : RATIO : ( RATELIMITER ~ _ ) : DB2COEFF;


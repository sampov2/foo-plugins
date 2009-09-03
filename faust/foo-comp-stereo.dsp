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

declare name      "foo schmoozcomp stereo";
declare author    "Sampo Savolainen";
declare version   "0.9b";
declare copyright "(c)Sampo Savolainen 2009";

import ("compressor-basics.dsp");


COMP =  DETECTOR : RATIO : ( RATELIMITER ~ _ ) : DB2COEFF ;

STEREO_SPLITTER(l, r) = ( (l + r) * 0.5 , l, r);
STEREO_GAIN(drywet, gain, l, r)      = (
	( l * (1.0 - (1.0 - gain) * drywet) ),
	( r * (1.0 - (1.0 - gain) * drywet) ));

process = STEREO_SPLITTER : ( COMP, _, _) : STEREO_GAIN(drywet);


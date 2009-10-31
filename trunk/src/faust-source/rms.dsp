// Taken from fausts benchmark/ directory. The original file did not include
// a copyright or license, so its assumed to be under the FAUST compiler license:
/**
    FAUST compiler, Version 0.9.3
        Copyright (C) 2003-2005 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

----------------------------------------------------------------------------
 **/

// Root Mean Square of n consecutive samples
RMS(n) = square : mean(n) : sqrt ;

// the square of a signal
square(x) = x * x ;

// the mean of n consecutive samples of a signal
// uses fixpoint to avoid the accumulation of
// rounding errors 
mean(n) = float2fix : integrate(n) : fix2float : /(n); 

// the sliding sum of n consecutive samples of a signal
integrate(n,x) = x - x@n : +~_ ;

// convertion between float and fix point
float2fix(x) = int(x*(1<<20));      
fix2float(x) = float(x)/(1<<20);    

// Root Mean Square of 1000 consecutive samples
//process = RMS(1000) ;

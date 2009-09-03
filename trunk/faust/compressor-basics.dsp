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

/**
 * gschmooz basics. this file includes all compressor functions except
 * for the final process() method.
 **/

import("rms.dsp");

SR = fconstant(int fSamplingFreq, <math.h>);

COEFF2DB(x) = log10(x) * 20;
DB2COEFF(x) = select2( (x < -318.8) , pow(10, x / 20), 0);
//DB2COEFF(x) = pow(10, x / 20);


/** start of RMS **/
/*
// TODO: does this need to be a "real" RMS?
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
*/
/** end of RMS **/


// apply a threshold to the value and then normalize it to 0
THRESH(t,x) = (x-t) * (t < x);

// smoothing function, attack coefficient "a", release coefficient "r" and signal x
SMOOTH(a, r, prevx, x) = 
	(x     *      select2( (x < prevx), a, r )) + 
	(prevx * (1 - select2( (x < prevx), a, r)));

// SR ..

DETECTOR = (	RMS(rms_speed) : 
		COEFF2DB :
		THRESH(threshold) : 
		SMOOTH(attack, release) ~ _ );

RATIO(x) = makeup_gain - (x - (x/ratio));

/**
 *  Ranges from TK audio BC1
 *
 * Attack  = 0.1ms .. 120ms
 * Release =  50ms .. 1.2s
 * 
 * High pass = 150Hz, -6dB / octave
 *      (one should probably make it switchable between 15Hz and 150Hz, just to 
 *       get rid of DC)
 */

/**
 * Time ratio (tr) to real time (t):
 * 
 *   tr = exp(1) / ( t * SR * time_ratio_target )
 *
 * The ratio system never comes up to the 'target value'. 
 * Assuming previous signal level 0.0 and target signal level 1.0
 *
 *  at time_ratio_target = 2,   after t, level will be 0.75
 *  at time_ratio_target = 1.5, after t, level will be 0.84
 *  at time_ratio_target = 1,   after t, level will be 0.94
 *  at time_ratio_target = 0.5, after t, level will be 0.995
 *
 * This ratio is one of the most important parts of the compressor as
 * it affects how release and attack timings are translated. 
 *  - The lower the ratio, the steeper the first part of the envelope will.
 *  - The higher the ratio, the action will be farther from what the user expects.
 *
 **/

time_ratio_target = 1.5; 

//time_ratio_target_atk = hslider("attack time ratio", 1.5, 0.2, 10.0, 0.1); 
//time_ratio_target_rel = hslider("release time ratio", 1.5, 0.2, 5.0, 0.1);
time_ratio_target_atk = 8.0;
time_ratio_target_rel = 4.0; // this could be too slow

time_ratio_attack(t) = exp(1) / ( t * SR * time_ratio_target_atk );
time_ratio_release(t) = exp(1) / ( t * SR * time_ratio_target_rel );


// TODO: RMS speed should be relative to SR!

rms_speed	 = 24; // ceil(SR * 0.003);  // SR...
threshold	 = hslider("threshold (dB)",         -10.0,  -60.0,   10.0, 1.0);
attack		 = time_ratio_attack( hslider("attack (ms)", 10.0,    0.1,  120.0, 0.1) / 1000 );
release		 = time_ratio_release( hslider("release (ms)", 300,     50, 1200.0, 1.0) / 1000 );

ratio		 = hslider("compression ratio",          5,    1.5,   20,   0.5);
makeup_gain 	 = hslider("makeup gain (dB)",           0,      0,   20,   0.5); // DB

drywet		 = hslider("dry/wet", 1.0, 0.0, 1.0, 0.1);


/*
// Hard-wired settings for sndfile tests
rms_speed   = 24;
threshold   = -10;
attack      = time_ratio_attack( 0.001 );
release     = time_ratio_release( 0.300 );
ratio       = 5;
makeup_gain = 0;
drywet      = 1.0;
*/


DRYWET(ratio) = ( *(1 - ratio),  * (ratio)) : +;
DRYWET_STEREO(l, r, ratio) = ( (DRYWET(l, ratio)), (DRYWET(r, ratio)));



//maximum_rate = 10.0/SR;
maximum_rate = 96.0/SR;
//maximum_rate = 512.0/SR;

RATELIMITER_INTERNAL(pt, ct, prevx, x) = 
     select2( abs(ct-pt) > maximum_rate, x, 
              prevx - pt + maximum_rate * select2( (x < prevx), 1.0, -1.0) );

// this corrects for overshooting caused by rate limiting. 
// in effect it should eliminate gain oscillation due to this overhsoot
OVERSHOOT_CORRECTION(limited_gain, target_gain, tangent) = 
	select2( (tangent > 0.0),
		 select2( (limited_gain > target_gain), limited_gain, target_gain),
		 select2( (limited_gain > target_gain), target_gain, limited_gain));

RATELIMITER(prevx, x) = 
	( RATELIMITER_INTERNAL( prevx@1 - prevx, prevx - x, prevx, x), x, (prevx - x) ) :
	  OVERSHOOT_CORRECTION;




/*

COMP = _ <: ( DETECTOR : RATIO : ( RATELIMITER ~ _ ) : DB2COEFF );

//COMP = _ <: ( DETECTOR : RATIO : DB2COEFF );

process =  _ <: ( _ , *(COMP) ) : DRYWET(drywet);

//process =  DETECTOR : RATIO : ( RATELIMITER ~ _ ) : DB2COEFF;

*/

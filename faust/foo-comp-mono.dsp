declare name      "foo schmoozcomp mono";
declare author    "Sampo Savolainen";
declare version   "0.9b";
declare copyright "(c)Sampo Savolainen 2009";

SR = fconstant(int fSamplingFreq, <math.h>);

/**
 * (stolen from TK audio BC1)
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
//time_ratio_target_atk = 2.5;
//time_ratio_target_atk = 25;
time_ratio_target_rel = 1.0;

time_ratio_attack(t) = exp(1) / ( t * SR * time_ratio_target_atk );
time_ratio_release(t) = exp(1) / ( t * SR * time_ratio_target_rel );


rms_speed	 = 24; // ceil(SR * 0.003);  // SR...
threshold	 = hslider("threshold (dB)",         -10.0,  -60.0,   10.0, 1.0);
attack		 = time_ratio_attack( hslider("attack (ms)", 10.0,    0.1,  120.0, 0.1) / 1000 );
release		 = time_ratio_release( hslider("release (ms)", 300,     50, 1200.0, 1.0) / 1000 );

ratio		 = hslider("compression ratio",          5,    1.5,   20,   0.5);
makeup_gain 	 = hslider("makeup gain (dB)",           0,      0,   20,   0.5); // DB

drywet		 = hslider("dry/wet", 1.0, 0.0, 1.0, 0.1);


/*
rms_speed   = 48;
threshold   = -10;
attack      = time_ratio_attack( 0.001 );
release     = time_ratio_release( 0.300 );
ratio       = 5;
makeup_gain = 0;
drywet      = 1.0;
*/

import ("compressor-basics.dsp");

DRYWET(ratio) = ( *(1 - ratio),  * (ratio)) : +;

/**
 * limit the rate the tangent of the signal changes.
 *
 *  -- THIS IS WRONG! --
 * previous tangent, pt: (prevx@1 - prevx)
 * current tangent,  ct: (prevx - x)
 *
 * if ( abs(ct-pt) > maximum_rate ) {
 *    x = prevx + maximum_rate * select2( (x < prevx) , 1.0, -1.0 );
 * }
 *  -- END OF WRONG --
 * 
 * previous slope: 
 * 
 * 
 * 
 **/

/*
//maximum_rate = 96.0/SR;
maximum_rate = 512.0/SR;
RATELIMITER_INTERNAL(pt, ct, prevx, x) = 
     select2( abs(ct-pt) > maximum_rate, x, 
              prevx - pt + maximum_rate * select2( (x < prevx), 1.0, -1.0) );

RATELIMITER(prevx, x) = RATELIMITER_INTERNAL( prevx@1 - prevx, prevx - x, prevx, x);
//COMP = _ <: ( DETECTOR : RATIO : ( RATELIMITER ~ _ ) : DB2COEFF );
*/

COMP = _ <: ( DETECTOR : RATIO : DB2COEFF );

process =  _ <: ( _ , *(COMP) ) : DRYWET(drywet);

//process =  DETECTOR : RATIO : ( RATELIMITER ~ _ ) : DB2COEFF;


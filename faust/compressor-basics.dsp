
COEFF2DB(x) = log10(x) * 20;
DB2COEFF(x) = select2( (x < -318.8) , pow(10, x / 20), 0);
//DB2COEFF(x) = pow(10, x / 20);

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

// Root Mean Square of 1000 consecutive samples

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

/*
STEREO_SPLITTER(l, r) = (l + r, l, r);
GAIN(gain, l, r)      = (gain * l, gain *r);
*/
// Stereo version
//process = STEREO_SPLITTER : (DETECTOR : RATIO ) : GAIN;

// Mono versio
//process =  _ <: (DETECTOR : RATIO ) * _;


//process =  DETECTOR : RATIO;




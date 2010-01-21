import ("math.lib");
import ("music.lib");

//import ("rc_filter.dsp");
import ("biquad.dsp");

sinc = _ <: select2( == (0), real_sinc, 1.0)
with{
	real_sinc = *(PI) <: sin / _;
};

blackman = *(2.0 * PI) <: 0.42 - 0.5 * cos(_) + 0.08 * cos(_*2.0);

at_frame = plus_one ~ _
with {
	plus_one = _+(1);
};

zero_to_one(n) = at_frame / n;

/*
blep(zeroCrossings, overSampling) = dft_sinc_atomic_sum
with {
	n = (zeroCrossings * 2 * overSampling) + 1;

	windowed_sinc(i) = rdtable(n, sinc_it * window_it, int(i));

	dft_sinc_atomic_sum(k) = 
		  sum(i, n, windowed_sinc * cos( (2 * PI * k * i) / n) )
		+ sum(i, n, windowed_sinc * sin( (2 * PI * k * i) / n) * -1 );


	//real_cepstrum = sum(k, n, sum(i, n, dft_sinc_atomic_sum(k, i) ) );

	//dft_sinc_atomic_sum(k, i) = windowed_sinc(
//	sinc_index(i) = -zeroCrossings + i/n * zeroCrossings * 2;
	

	sinc_it = sinc( -zeroCrossings + (_)/n * zeroCrossings * 2);
	window_it = blackman( _ / n);
};
*/

//zeroCrossings = 32;
//overSampling  = 72;

zeroCrossings = 4;
overSampling  = 4;
	n = (zeroCrossings * 2 * overSampling) + 1;

blep(i) = select2(i < n-1, 1.0, blep_table_lookup(i))
with {

	inc_by_one = _ ~ _+1 : -(1);

	ext_blep = ffunction( float get_blep(int, int, int) , "blep.cpp", "");

	blep_table = rdtable(n, ext_blep(zeroCrossings, overSampling, inc_by_one), int );


	linear_interpolation(a, b, r) = a + r * (b-a); //, a, b, r;

	blep_table_lookup(x) = 
		linear_interpolation ( blep_table( floor(x)      ),
                                       blep_table( floor(x) + 1  ),
                                       fmod(x, 1.0) );
};

blep_naive_square(freq) = blep(phase * n)
with {
	wavelength = float(SR)/float(freq);
	q = 1.0/wavelength;

	plusone = _ + 1;
	phase = (plusone ~ fmod(_, wavelength)) / wavelength;

	blop(ph) = 1;

	the_osc = phase <:
		select2( _ >= 0.5,
		blep(_      ) +
		blep(_ + q  ) +
		blep(_ + q*2) +
		blep(_ + q*3)
		,
		-1*(
		blep( fmod(_ + 0.5      , 1.0)) + 
		blep( fmod(_ + 0.5 + q  , 1.0)) +
		blep( fmod(_ + 0.5 + q*2, 1.0)) +
		blep( fmod(_ + 0.5 + q*3, 1.0))));
/*
	the_osc(lastblep, prev) = ..?
blep_naive_square(freq) = the_osc ~ (_,_) : (!, !, _)
		blop(select2(phase < 0.5, phase-0.5, phase)) <:
			 _, 
			( *(select2(phase < 0.5, -1, 1)) : -(lastblep) : +(prev) <: (_, _));

	*/

};

blit_saw(freq) = (blit_it ~ _) : biquad_hp(freq/4.0)
with {
	phases = 32;
	blit = _ <: blackman * sinc( (_ - 0.5)*phases );
	phase = +(float(freq)/float(SR));

	blit_it(prev) = (phase ~ fmod(_,1.0)) <: prev + blit; //- _*(0.015);
};

blit_square(freq) = (blit_it ~ _) 
with {
	phases = 6;
	//phases = vslider("phases", 32, 4, 128, 1);
	blit = _ <: blackman * sinc( (_ - 0.5)*phases );
	phase = +(float(freq)/float(SR));

	sincs = 4;

	blit_it(prev) = (phase ~ fmod(_,1.0)) <: prev + 
		sum(i, 5, blit(fmod(_,       1.0) + (i-3) )) - 
		sum(i, 5, blit(fmod(_ + 0.5, 1.0) + (i-3) ));
};

polyblep_square(freq) = (phase ~ _) : naive_square : (polyb_it ~ _) : (!, _)
with {
	q = float(freq)/float(SR);
	phase = +(q) : fmod(_, 1.0);

	polyblep_real(t) = 
		select2(t < -1, 
			select2(t > 1, 
				select2( t > 0, 
					((t*t)/2 + t + 0.5), 
					(t - (t*t)/2 - 0.5))
				,0)
			,0);

	polyblep(t) = polyblep_real( t / q) * 2;

	naive_square(ph) = ph, select2( ph < 0.5, -1.0, 1.0);

	// 0 no polyblep
	// 1 add polyblep at ph
	// 2 add reverse phase polyblep at ph - 0.5
	selector(ph) = select2( (ph - ph') <= 0, 
		       select2( fmod(ph+0.5,1.0) < q, 0, 2), 1);

	// Detects square wave discontinuities by checking whether phase has went past 0 or 0.5
	// delays signal by one.
	polyb_it(prev, ph, x) = 
		select3(selector(ph),
			x,
			x + polyblep(ph) - 0,
			x - polyblep(ph - 0.5) - 0), 
		select3(selector(ph), 
			prev, 
			prev + polyblep(ph' - 1.0) + 0, 
			prev - polyblep(ph' - 0.5) + 0);

};
//process = polyblep_square(440.3);
//process = polyblep_square(8123.1);
//process = polyblep_square(5053);
//process = polyblep_square(2399) * 0.8;
process = polyblep_square(3726);
//process = polyblep_square(vslider("frequency", 440.0, 10.0, 30000.0, 5.0)) * 0.2;

	polyblep_uff(t) = 
		select2(t < -1, 
			select2(t > 1, 
				select2( t > 0, 
					((t*t)/2 + t + 0.5), 
					(t - (t*t)/2 - 0.5))
				,0)
			,0);
//process = zero_to_one(1024) * 2 - 1.5 : polyblep_uff; // : polyblep_square(440.0);
/*
with {
	a = (t*t)/2 + t + 0.5;
	b = t - (t*t)/2 + 0.5;	
};
*/
//process = zero_to_one(1024) * 2 - 1 : fmod(_, 1.0);



//process = blit_square(440.4*8); // 8123.4
//process = blit_square(vslider("frequency", 8123.4, 10.0, 30000.0, 5.0));

//blit = _ <: blackman * sinc( _*phases - (phases/2) )
//process = blit(vslider("frequency", 8123.4, 10.0, 30000.0, 5.0));

//process = at_frame : -(100) : /(12) : sinc;
//process = zero_to_one(1000) : blackman;

/*
gulash = rdtable(100, sin, index(i))
with {
	index(x) = int(fmod(x, 100));
};
process = gulash;
*/


//doodaa(i) = 1/(i+10);

//process = par(i, 4, doodaa(i));
//process = par(i, 4, doodaa(i)) + par(i, 4, doodaa(i-20));
//process = sum(k, 2, par(i, 4, doodaa(i-20*(k-1))));


//process = float(at_frame-1) : /(1.32) : blep(5, 16);
//process = float(at_frame-1) : /(1) : blep;

//process = blep_naive_square(4);



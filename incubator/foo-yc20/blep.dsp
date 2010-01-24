import ("math.lib");
import ("music.lib");

at_frame = plus_one ~ _
with {
	plus_one = _+(1);
};

zero_to_one(n) = at_frame / n;

fast_polyblep(t) = polyblep_real(t)
with {
	table_size = 16384;

	polynomial(t) = select2( t > 0, 
				((t*t)/2 + t + 0.5), 
				(t - (t*t)/2 - 0.5));

	polynomial_from_idx(i) = i*2.0/table_size-1 : polynomial;

	polyblep_table(i) = rdtable(table_size, polynomial_from_idx, +(1) ~ _ : -(1));

	polyblep_real(t) = (t+1)/2.0*float(table_size) <: (polyblep_table(floor) + polyblep_table(ceil))/2;
};

polyblep_square_master(f, bias) = (phase ~ _) <: polyblep_square_slave, _
with {
	freq = f * bias;

	q = float(freq)/float(SR);
	phase = +(q) : fmod(_, 1.0);
};

polyblep_square_slave(phase) = naive_square(phase) : (polyb_it ~ _) : (!, _)
with {

	q = fmod(phase - phase' + 1, 1.0);

	polyblep_real(t) = select2( t > 0, 
				   ((t*t)/2 + t + 0.5), 
				   (t - (t*t)/2 - 0.5));

	//polyblep(t) = polyblep_real( t / q);
	polyblep(t) = fast_polyblep( t / q);

	naive_square(ph) = ph, select2( ph < 0.5, -1.0, 1.0);

	// 0 no polyblep
	// 1 add polyblep at ph
	// 2 add reverse phase polyblep at ph - 0.5
	selector(ph) = select2( (ph - ph') <= 0, 
		       select2( fmod(ph+0.5,1.0) < q, 0, 2), 1);

	// Detects square wave discontinuities by checking whether phase has went past 0 or 0.5
	// delays signal by one.
	polyb_it(prev, ph, x) = 
		selector(ph) <:
		select3(_,
			x,
			x + polyblep(ph) * 2,
			x - polyblep(ph - 0.5) * 2), 
		select3(_,
			prev, 
			prev + polyblep(ph' - 1.0) * 2, 
			prev - polyblep(ph' - 0.5) * 2);

};



//process = zero_to_one(2048) * 2 - 1 : fast_polyblep;

//process = fast_polyblep( 2.4 / 1923.4);

phase_divisor(ph) = slow_accumulator(ph) / 2.0
with {
        slow_accumulator(x) = (prevphase(x) ~ _) + x;

        prevphase(x, whichphase) = select2( (x - x@1) < 0, whichphase, 1-whichphase);
};

process = polyblep_square_master(3440.1, 1.0) : _, (phase_divisor : polyblep_square_slave);


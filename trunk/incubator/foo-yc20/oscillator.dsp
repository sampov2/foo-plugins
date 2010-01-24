// almost right, but it starts from the wrong note
tet12(note) = 440*8 * 2.0^((note + 1) / 12.0);

oscillators = par(i, 12, polyblep_square_master(tet12(i)));

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

	// with polyblep_real() it works, with fast_polyblep() the compiler just spews out:
	//   Error : checkInit failed for type RSESN interval()
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

fast_polyblep(t) = polyblep_lookup(t)
with {
	table_size = int(16384);

	polynomial(t) = select2( t > 0, 
				((t*t)/2 + t + 0.5), 
				(t - (t*t)/2 - 0.5));

	polynomial_from_idx(i) = float(i)*2.0/float(table_size)-1.0 : polynomial;

	polyblep_table(i) = rdtable(table_size, polynomial_from_idx, int(+(1) ~ _ : -(1)));

	polyblep_lookup(t) = (t+1)/2.0*float(table_size) <: (polyblep_table(floor:int) + polyblep_table(ceil:int))/2;
};



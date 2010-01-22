// almost right, but it starts from the wrong note
tet12(note) = 440*8 * 2.0^((note + 1) / 12.0);

oscillators = par(i, 12, polyblep_square_master(tet12(i)));

polyblep_square_master(f, bias) = (phase ~ _) <: polyblep_square_slave, _
with {
	freq = f * bias;

	q = float(freq)/float(SR);
	phase = +(q) : fmod(_, 1.0);
};

polyblep_square(f, bias) = (phase ~ _) : naive_square : (polyb_it ~ _) : (!, _)
with {
	freq = f * bias;

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

	polyblep(t) = polyblep_real( t / q);

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
			x + polyblep(ph) * 2,
			x - polyblep(ph - 0.5) * 2), 
		select3(selector(ph), 
			prev, 
			prev + polyblep(ph' - 1.0) * 2, 
			prev - polyblep(ph' - 0.5) * 2);

};

polyblep_square_slave(phase) = naive_square(phase) : (polyb_it ~ _) : (!, _)
with {

	q = fmod(phase - phase' + 1, 1.0);

	polyblep_real(t) = 
		select2(t < -1, 
			select2(t > 1, 
				select2( t > 0, 
					((t*t)/2 + t + 0.5), 
					(t - (t*t)/2 - 0.5))
				,0)
			,0);

	polyblep(t) = polyblep_real( t / q);

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
			x + polyblep(ph) * 2,
			x - polyblep(ph - 0.5) * 2), 
		select3(selector(ph), 
			prev, 
			prev + polyblep(ph' - 1.0) * 2, 
			prev - polyblep(ph' - 0.5) * 2);

};


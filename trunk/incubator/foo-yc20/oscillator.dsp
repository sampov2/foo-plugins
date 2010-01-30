// almost right, but it starts from the wrong note
tet12(note) = 440*8 * 2.0^((note + 1) / 12.0);

oscillators = par(i, 12, polyblep_square_master(tet12(i)));

polyblep_square_master(f, bias) = (phase ~ _) <: polyblep_square_slave, _
with {
	freq = f * bias;

	q = float(freq)/float(SR);
	phase = +(q) : modone;
};

polyblep_square_slave(phase) = naive_square(phase) : (polyb_it ~ _) : (!, _)
with {

	q = modone(phase - phase' + 1);

	naive_square(ph) = ph, select2( ph < 0.5, -1.0, 1.0);

	// 0 no polyblep
	// 1 add polyblep at ph
	// 2 add reverse phase polyblep at ph - 0.5
	selector(ph) = select2( ph' > ph, 
		       select2( modone(ph+0.5) < q, 0, 2), 1);

	// Detects square wave discontinuities by checking whether phase has went past 0 or 0.5
	// delays signal by one.
	polyb_it(prev, ph, x) = 
		selector(ph) <:
			square_blep_x   (_, x,    ph,  q) ,
			square_blep_prev(_, prev, ph', q);

	square_blep_x    = ffunction (float square_blep_x   (int, float, float, float), "polyblep.cpp", "");
	square_blep_prev = ffunction (float square_blep_prev(int, float, float, float), "polyblep.cpp", "");
};


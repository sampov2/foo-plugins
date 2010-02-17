wave_transformers = 
	wave_transformer_I1,
	wave_transformer_I1,
	wave_transformer_I1,
	wave_transformer_I1,
	wave_transformer_I2,
	wave_transformer_I2,
	wave_transformer_I2,
	wave_transformer_I2,
	wave_transformer_I3,
	wave_transformer_I3,
	wave_transformer_I3,
	wave_transformer_I3;

wave_transformer(C6, C5, C4, C3, C2, C1, C0) = (
	passive_hp(bus_bar_impedance, 0.039),
	(lopass(C6) : hipass(bus_bar_impedance)),
	(lopass(C5) : hipass(bus_bar_impedance)),
	(lopass(C4) : hipass(bus_bar_impedance)),
	(lopass(C3) : hipass(bus_bar_impedance)),
	(lopass(C2) : hipass(R2)),
	(lopass(C1) : hipass(R1)),
	(lopass(C0) : hipass(R0))
	)
with {
	//lopass(C) = passive_lp(2500, C);
	//lopass(C) = biquad_lp(400);

	// looked good on the fft, but sounded wrong..
	//lopass(C) = _ <: passive_lp(input_impedance, C) + _*0.29;

	// empirically chosen, good with at least one note and 15k input_impedance :)
	//lopass(C) = _ <: passive_lp(input_impedance, C) + _* 0.015;
	//hipass(R) = passive_hp(R + input_impedance, 0.039);

	// another go..
	lopass(C) = _ <: passive_lp(input_impedance, C);

	hipass(R) = passive_hp(R, 0.039);

	input_impedance = 15000;

	bus_bar_impedance   = 180000.0 + 1.0/( 1.0/10000.0 + 1.0/100000.0 );
	// bus bar 16 has a 15K resistor instead of 10K, but we can't go to that level
	// of detail, at least yet.

	// bus bar impedance affects the high pass cutoff point
	R2 = 1.0 / ( 1.0/bus_bar_impedance + 1.0/180000.0 );
	R1 = 1.0 / ( 1.0/bus_bar_impedance + 1.0/82000.0  );
	R0 = 1.0 / ( 1.0/bus_bar_impedance + 1.0/56000.0  );
	
};


//                                      C6      C5      C4     C3     C2     C1     C0
// C, C#, D, D#
wave_transformer_I1 = wave_transformer(0.0047, 0.01,   0.022, 0.047, 0.082, 0.12,  0.15);

// E, F, F#, G
wave_transformer_I2 = wave_transformer(0.0039, 0.0082, 0.018, 0.039, 0.068, 0.1,   0.15);

// G#, A, A#, B
wave_transformer_I3 = wave_transformer(0.0027, 0.0056, 0.012, 0.027, 0.056, 0.082, 0.12);



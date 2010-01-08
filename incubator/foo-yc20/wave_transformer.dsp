

passive_rc_freq(r, uf) = 1/(2 * PI * r * capacitance)
with {
	capacitance = uf / 1000000;
};


passive_lp(r, uf) = biquad_lp(passive_rc_freq(r,uf));
passive_hp(r, uf) = biquad_hp(passive_rc_freq(r,uf));


//wave_transformers = wave_transformer_I1_C : par(i, 11, wave_transformer);
//wave_transformers = par(i, 12, wave_transformer);
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
	// passive lp on the 1st drawbar is to emulate capacitance in the circuitry.
	// otherwise it will be too shrill
	//(passive_hp(next_stage_resistance, 0.039) : passive_lp(next_stage_resistance, 0.0002 )),
	passive_hp(next_stage_resistance, 0.039),
	(passive_lp(15000, C6) : passive_hp(next_stage_resistance + 15000, 0.039)),
	(passive_lp(15000, C5) : passive_hp(next_stage_resistance + 15000, 0.039)),
	(passive_lp(15000, C4) : passive_hp(next_stage_resistance + 15000, 0.039)),
	(passive_lp(15000, C3) : passive_hp(next_stage_resistance + 15000, 0.039)),
	(passive_lp(15000, C2) : passive_hp(R2 + 15000, 0.039)), 
	(passive_lp(15000, C1) : passive_hp(R1 + 15000, 0.039)), 
	(passive_lp(15000, C0) : passive_hp(R0 + 15000, 0.039)) 
	)
with {

	// TODO: This is still a guess. It could probably be looked up on the schematics
	next_stage_resistance = 180000.0 + 1.0/( 1.0/10000.0 + 1.0/100000.0 );

	// next stage resistance affects the high pass cutoff point
	R2 = 1.0 / ( 1.0/next_stage_resistance + 1.0/180000.0 );
	R1 = 1.0 / ( 1.0/next_stage_resistance + 1.0/82000.0  );
	R0 = 1.0 / ( 1.0/next_stage_resistance + 1.0/56000.0  );
	
};


//                                      C6      C5      C4     C3     C2     C1     C0
// C, C#, D, D#
wave_transformer_I1 = wave_transformer(0.0047, 0.01,   0.022, 0.047, 0.082, 0.12,  0.15);

// E, F, F#, G
wave_transformer_I2 = wave_transformer(0.0039, 0.0082, 0.018, 0.039, 0.068, 0.1,   0.15);

// G#, A, A#, B
wave_transformer_I3 = wave_transformer(0.0027, 0.0056, 0.012, 0.027, 0.056, 0.082, 0.12);



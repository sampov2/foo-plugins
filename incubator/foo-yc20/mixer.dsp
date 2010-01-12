
balance_control = hslider("balance", 1.0, 0.0, 1.0, 0.25);

manual_i_1     = hgroup("i", vslider("[7]1'",     0.5,  0.0, 1.0, 0.25));
manual_i_1_3p5 = hgroup("i", vslider("[6]1 3/5'", 0.0,  0.0, 1.0, 0.25));
manual_i_2     = hgroup("i", vslider("[5]2'",     1.0,  0.0, 1.0, 0.25));
manual_i_2_2p3 = hgroup("i", vslider("[4]2 2/3'", 0.5,  0.0, 1.0, 0.25));
manual_i_4     = hgroup("i", vslider("[3]4'",     1.0,  0.0, 1.0, 0.25));
manual_i_8     = hgroup("i", vslider("[2]8'",     1.0,  0.0, 1.0, 0.25));
manual_i_16    = hgroup("i", vslider("[1]16'",    0.5,  0.0, 1.0, 0.25));

manual_ii_2    = hgroup("ii", vslider("[4]2'",    1.0,  0.0, 1.0, 0.25));
manual_ii_4    = hgroup("ii", vslider("[3]4'",    1.0,  0.0, 1.0, 0.25));
manual_ii_8    = hgroup("ii", vslider("[2]8'",    1.0,  0.0, 1.0, 0.25));
manual_ii_16   = hgroup("ii", vslider("[1]16'",   1.0,  0.0, 1.0, 0.25));

manual_bass_8  = hgroup("bass", vslider("[2]8'",  1.0,  0.0, 1.0, 0.25));
manual_bass_16 = hgroup("bass", vslider("[1]16'", 1.0,  0.0, 1.0, 0.25));

mixer = mixer_normal, mixer_bass :> +(_) : *(0.01 + 0.2 * hslider("volume", 0.1, 0.0, 1.0, 0.1));
mixer_normal (bus_1, bus_1_3p5, bus_2, bus_2_2p3, bus_4, bus_8, bus_16) = balance(manual_i, manual_ii)
with {
	balance = (_ * balance_control) + (_ * (balance_control-1));

	manual_i = bus_1     * manual_i_1
		 + bus_1_3p5 * manual_i_1_3p5
		 + bus_2     * manual_i_2
		 + bus_2_2p3 * manual_i_2_2p3
		 + bus_4     * manual_i_4
		 + bus_8     * manual_i_8
		 + bus_16    * manual_i_16;

	manual_ii = bus_2    * manual_ii_2
		  + bus_4    * manual_ii_4
		  + bus_8    * manual_ii_8
		  + bus_16   * manual_ii_16;
	// TODO: filter & brightness
};


mixer_bass (bass_bus_4, bass_bus_8, bass_bus_16) = mix : filter : gain
with {
	mix = manual_bass_8  * (bass_bus_8  + bass_bus_4 * 0.5)
	    + manual_bass_16 * (bass_bus_16 + bass_bus_8 * 0.5 + bass_bus_4 * 0.25);

	// This filter is tricky, the different buses are mixed with different
	// resistors plus the mix potentiometers are connected so that they vary
	// impedance to the filter. 10k is just a random stab in the (silent) dark.
	filter = passive_lp(10000, 0.056);

	// This should compensate for the relative quietness, also TODO: Bass volume!
	gain = *(6.0);
};


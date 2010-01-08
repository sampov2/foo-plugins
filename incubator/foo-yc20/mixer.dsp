
balance_control = vslider("balance", 1.0, 0.0, 1.0, 0.25);

manual_i_1     = hgroup("i", vslider("1'", 1.0, 0.0, 1.0, 0.25));
manual_i_1_3p5 = hgroup("i", vslider("1 3/5'", 1.0, 0.0, 1.0, 0.25));
manual_i_2     = hgroup("i", vslider("2'", 1.0, 0.0, 1.0, 0.25));
manual_i_2_2p3 = hgroup("i", vslider("2 2/3'", 1.0, 0.0, 1.0, 0.25));
manual_i_4     = hgroup("i", vslider("4'", 1.0, 0.0, 1.0, 0.25));
manual_i_8     = hgroup("i", vslider("8'", 1.0, 0.0, 1.0, 0.25));
manual_i_16    = hgroup("i", vslider("16'", 1.0, 0.0, 1.0, 0.25));

mixer = mixer_normal, mixer_bass :> +(_) : *(0.1);

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

	manual_ii = 0;
};


mixer_bass (bass_bus_4, bass_bus_8, bass_bus_16) = (0.0);


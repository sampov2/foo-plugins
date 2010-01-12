
// Thanks to Torben Hohn and...
// http://www.dsplog.com/2007/12/02/digital-implementation-of-rc-low-pass-filter/

passive_lp(resistance, uf) = (filter ~ _)
with {
	// simplified to minimize floating point precision problems
	filter(prev,x) = prev + (( x - prev) * 1000000) / (resistance * uf * SR);

	// readable implementation
	//filter(prev,x) = prev + k * ( x - prev);
	//k = 1 / RC * dt;
	//RC = resistance * uf / 1000000;
	//dt = 1 / SR;
};

passive_hp(resistance, uf) = (filter ~ _)
with {

	filter(prev,x) = alpha * prev + alpha * (x - x');

	alpha = RC / (RC + dt);

	RC = resistance * uf / 1000000;
	dt = 1 / SR;
};

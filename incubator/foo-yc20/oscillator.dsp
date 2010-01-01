


oscillators = par(i, 12, oscillator(440*9 * 2.0^(i / 12.0)));

oscillator(f, bias) = sine_wave_square_wave_oscillator(f * bias)
with {
	// how many samples is each full waveform
	wavelength(freq) = (float)(freq)/(float)(SR);

	// keeps the relative position of a waveform
	position_at_wave(freq,c) = wavelength(freq) + c;

	sine_wave_square_wave_oscillator(freq) = 
		sines_for_relative_position((position_at_wave(freq) ~ fmod(_,1.0)));

	sines_for_relative_position(pos) =
		sin ( pos     * 2 * PI )     +
		sin ( pos * 3 * 2 * PI ) / 3 +
		sin ( pos * 5 * 2 * PI ) / 5 +
		sin ( pos * 7 * 2 * PI ) / 7;

};

// kept here for good measure

oscillator_new(f, bias) = sine_wave_oscillator(f * bias) : biquad_hp(f * bias) : cutup
with {
	// how many samples is each full waveform
	wavelength(freq) = (float)(freq)/(float)(SR);

	// keeps the relative position of a waveform
	position_at_wave(freq,c) = wavelength(freq) + c;

	// square wave so half of the waveform is at 1.0 and the other half at -1.0
	sine_wave_oscillator(freq) = sin ( (position_at_wave(freq) ~ fmod(_,1.0)) * 2 * PI );

	cutup(x) = select2( x > 0.0, 1.0, select2(x < 1.0, 0.0, -1.0));

};

// really a square wave
// the oscillator function where bias is taken into account
oscillator_old(f, bias) = square_wave_oscillator(f * bias)
with {
	// how many samples is each full waveform
	wavelength(freq) = noise*oscillator_dither_amount + (float)(freq)/(float)(SR);

	// keeps the relative position of a waveform
	position_at_wave(freq,c) = wavelength(freq) + c;

	// square wave so half of the waveform is at 1.0 and the other half at -1.0
	square_wave_oscillator(freq) = select2( ( position_at_wave(freq) ~ fmod(_,1.0) ) >= 0.5, 1.0, -1.0);

//	freq_with_noise =  *(1 + (noise-0.5)*oscillator_dither_amount);
};


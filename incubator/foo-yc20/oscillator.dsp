


oscillators = par(i, 12, oscillator(440*9 * 2.0^(i / 12.0)));

// the oscillator function where bias is taken into account
oscillator(f, bias) = square_wave_oscillator(f * bias) : biquad_hp(f * bias)
with {
	// how many samples is each full waveform
	wavelength(freq) = (float)(freq)/(float)(SR);

	// keeps the relative position of a waveform
	position_at_wave(freq,c) = wavelength(freq) + c;

	// square wave so half of the waveform is at 1.0 and the other half at -1.0
	square_wave_oscillator(freq) = select2( ( position_at_wave(freq) ~ fmod(_,1.0) ) >= 0.5, 1.0, -1.0);

//	freq_with_noise(f) = f * (1 + (noise-0.5)*oscillator_dither_amount);
//	freq_with_noise(f) = f;
};


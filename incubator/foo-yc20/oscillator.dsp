


oscillators = par(i, 12, oscillator(440*8 * 2.0^(i / 12.0)));


// the oscillator function where bias is taken into account
oscillator(f, bias)=square_wave_oscillator(f * bias)
with {
	// how many samples is each full waveform
	samples_per_wave(freq) = SR/freq;

	// keeps the relative position of a waveform
	position_at_wave(freq,c) = 1.0 / samples_per_wave(freq) + c : fmod(_, 1.0);

	// square wave so half of the waveform is at 1.0 and the other half at -1.0
	square_wave_oscillator(freq) = select2( ( position_at_wave(freq) ~ _) > 0.5, 1.0, -1.0);
};


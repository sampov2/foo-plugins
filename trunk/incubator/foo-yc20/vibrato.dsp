

// Vibrato is from about 5hz to 8hz

vibrato = vibrato_speed * vibrato_amount
with {
	control_speed  = vgroup("[2]vibrato", hslider("[2]speed", 0.0, 0.0, 1.0, 0.2));
	control_amount = vgroup("[2]vibrato", hslider("[1]depth", 0.0, 0.0, 1.0, 0.2));

	// how many samples is each full waveform
	wavelength(freq) = (float)(freq)/(float)(SR);

	// keeps the relative position of a waveform
	position_at_wave(freq,c) = wavelength(freq) + c;

	// square wave so half of the waveform is at 1.0 and the other half at -1.0
	sine_wave_oscillator(freq) = sin ( (position_at_wave(freq) ~ fmod(_,1.0)) * 2 * PI );

	vibrato_speed = sine_wave_oscillator(5.0 + 3*control_speed);

	vibrato_amount = 0.0001 + 0.0199 * control_amount;
};
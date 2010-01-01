

// how many samples is each full waveform
samples_per_wave(freq) = SR/freq;

// keeps the relative position of a waveform
position_at_wave(freq) = fmod( 1.0 / samples_per_wave(freq) : + ~_, 1.0);

// square wave so half of the waveform is at 1.0 and the other half at -1.0
square_wave_oscillator(freq) = select2( position_at_wave(freq) > 0.5, 1.0, -1.0);

// the oscillator function where bias is taken into account
oscillator(freq, bias)=square_wave_oscillator(freq * bias);




percussion_envelope = detect : apply_envelope
with {
	rms(n) = square : mean(n) : sqrt ;
	square(x) = x * x;

	mean(n) = float2fix : integrate(n) : fix2float : /(n);

	integrate(n,x) = x - x@n : +~_ ;

	float2fix(x) = int(x*(1<<20));
	fix2float(x) = float(x)/(1<<20);



	rms_detect_speed = int(max(22050,min(192000,SR)) * 0.035);

	threshold = 0.3;

	detect = rms(rms_detect_speed) : detect_rise;

	detect_rise(sig) =
		select2( sig  > threshold, 0,
		select2( sig' < threshold, 0, 1));


	envelope_speed = 1.1;
	envelope_coeff = envelope_speed : exp(1) / (1.5 * float(SR) * _) : (1 - _);
	envelope_rms = int(max(22050,min(192000,SR)) * 0.005);

	apply_envelope = +(_*(1-envelope_coeff)) ~ *(envelope_coeff) 
			: *(1/(1-envelope_coeff)) 
			: rms(envelope_rms) : max(1.0);
};

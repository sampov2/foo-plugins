
// 8 dividers => 9 octaves

dividers = par(i, 12, divider) 
with {
	divider = _ <: _, 
		(flipflop <: _, 
		(flipflop <: _, 
		(flipflop <: _, 
		(flipflop <: _, 
		(flipflop <: _, 
		(flipflop <: _, 
		(flipflop <: _, 
		 flipflop
		)))))));
	flipflop = (flipflop_internal ~ _) : +(0.5) : *(2.0);
	flipflop_internal(prev, x) = prev - select2( x == 1, 0, select2(x@1 == -1, 0, 2*prev + 1 ));
};


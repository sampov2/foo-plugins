

flipflop_internal(prev, x) = prev - select2( x == 1, 0, select2(x@1 == -1, 0, 2*prev + 1 ));

flipflop = (flipflop_internal ~ _) : +(0.5) : *(2.0);

divider = _ <: _, (flipflop <: _, (flipflop <: _, flipflop <: _, flipflop));



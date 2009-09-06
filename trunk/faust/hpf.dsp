/*
 *  Copyright (C) 2009 Sampo Savolainen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */


// Note: Here a & b are switched when compared to http://en.wikipedia.org/wiki/IIR (Sep 6 2009)

/**
 * Test coefficients from: http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFiltDes102.html
 **/


// ********* Common:
iir_negate_and_divide(fb, ff) = (ff - fb) / b0;

// ********* IIR 2nd order:
iir_2nd_order_ff(x) = (a0 * x) + (a1 * x@1) + (a2 * x@2);
iir_2nd_order_fb(y) = (b1 * y) + (b2 * y@1);

iir_2nd_order = (iir_2nd_order_fb, iir_2nd_order_ff) :> iir_negate_and_divide;

// roundabouts 150Hz @ 48kHz
a0 = 0.9821821;
b0 = 1.0;
a1 = -1.9643642;
b1 = -1.9859269;
a2 = 0.9821821;
b2 = 0.9860424;

process = (iir_2nd_order ~ _);

// ********* IIR 8th order: (it does not work for some strange reason..)
iir_8th_order_ff(x) = (a0 * x) + (a1 * x@1) + (a2 * x@2) + (a3 * x@3) + (a4 * x@4) + (a5 * x@5) + (a6 * x@6) + (a7 * x@7) + (a8 * x@8);
iir_8th_order_fb(y) = (b1 * y) + (b2 * y@1) + (b3 * y@2) + (b4 * y@3) + (b5 * y@4) + (b6 * y@5) + (b7 * y@6) + (b8 * y@7);

iir_8th_order = (iir_8th_order_fb, iir_8th_order_ff) :> iir_negate_and_divide;

/*
a0 = 0.68044496;
b0 = 1.0;
a1 = -5.4435596;
b1 = -7.2508917;
a2 = 19.05246;
b2 = 23.038786;
a3 = -38.10492;
b3 = -41.89628;
a4 = 47.631145;
b4 = 47.69281;
a5 = -38.10492;
b5 = -34.801468;
a6 = 19.05246;
b6 = 15.89742;
a7 = -5.4435596;
b7 = -4.1567464;
a8 = 0.68044496;
b8 = 0.47636974;

process = (iir_8th_order ~ _);
*/

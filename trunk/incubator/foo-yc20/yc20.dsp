// tests to emulate a Yamaha YC-20 Combo Organ from the early 1970s

//declare name      "FooC-20";
declare name      "osc";
declare author    "Sampo Savolainen";
declare license   "GPL";
declare copyright "(c)Sampo Savolainen 2009";


import("music.lib");

import ("oscillator.dsp");
import ("divider.dsp");

/*

 Architecture & signal flow

   Master pitch + vibrato oscillator -> oscillator bias


   12x oscillators -> 12x flip-flop dividers -> 12*y (y = about 8) filters -> bus bars


 */

// bias should be affected by:
//  master pitch
//  vibrato (speed and depth controls)
//  touch vibrato
oscillator_bias = 1.0;

base_freq = 440*8;

process = oscillator_bias <: par(i, 12, oscillator(base_freq+i*base_freq/12 )) : par(i, 12, divider);


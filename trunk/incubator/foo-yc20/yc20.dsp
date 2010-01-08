// tests to emulate a Yamaha YC-20 Combo Organ from the early 1970s

//declare name      "FooC-20";
declare name      "osc";
declare author    "Sampo Savolainen";
declare license   "GPL";
declare copyright "(c)Sampo Savolainen 2009";


import("music.lib");

import ("biquad.dsp");

import ("oscillator.dsp");
import ("divider.dsp");
import ("wave_transformer.dsp");
import ("keyboard.dsp");
import ("mixer.dsp");

/*

 Architecture & signal flow

   Master pitch + vibrato oscillator -> oscillator bias


   12x oscillators -> 12x flip-flop dividers -> 12*y (y = about 8) filters -> bus bars

 */

// bias should be affected by:
//  master pitch
//  vibrato (speed and depth controls)
//  touch vibrato
oscillator_bias = vslider("oscillator bias",1.0, 0.1, 2.0, 0.001);


gain = par(i, 12*8, *(0.5));

process = oscillator_bias <: oscillators : dividers : wave_transformers : keyboard_slow : mixer;


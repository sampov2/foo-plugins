#!/bin/sh

echo '
plot \
 "plot/architect_input.txt"    with lines title "input signal",  \
 "plot/architect_fast_avg.txt" with lines title "fast envelope",  \
 "plot/architect_slow_avg.txt" with lines title "slow envelope"
' | gnuplot -persist


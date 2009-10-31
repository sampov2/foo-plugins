#!/bin/sh

#echo '
#plot \
# "plot/max_peak.txt"  with lines title "max peak", \
# "plot/input.txt"     with lines title "data in",  \
# "plot/output.txt"    with lines title "data out",  \
# "plot/envelope.txt"  with lines title "envelope"
#' | gnuplot -

gnuplot limiter.gnuplot -


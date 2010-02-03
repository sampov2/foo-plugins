

all: plot-c-osc

plot-c-osc: c-osc.data c-osc.plot
	gnuplot c-osc.plot -

c-osc.data: c-osc.wav c-osc.octave
	octave --silent c-osc.octave

c-osc.octave: octave_template
	(echo 'basename = "c-osc";'; cat octave_template) > c-osc.octave

c-osc.plot: gnuplot_template Makefile
	#MAXSAMPLE=`(echo -n '20*l('; cat c-osc.data | sort -n | tail -1 | tr -d '\n '; echo ')/l(10)') | bc -l`
	#SAMPLERATE=`sndfile-info c-osc.wav | grep ^"Sample Rate" | cut -d ':' -f 2 | tr -d ' '`
	#DATALENGTH=`wc -l c-osc.data  | cut -d ' ' -f 1`

	(cat gnuplot_template | sed \
		-e 's/DATAFILE/c-osc.data/' \
		-e 's/SAMPLERATE/'`sndfile-info c-osc.wav | grep ^"Sample Rate" | cut -d ':' -f 2 | tr -d ' '`'/' \
		-e 's/DATALENGTH/'`wc -l c-osc.data  | cut -d ' ' -f 1`'/' \
		-e 's/MAXSAMPLE/'`(echo -n '20*l('; cat c-osc.data | sort -n | tail -1 | tr -d '\n '; echo ')/l(10)') | bc -l`'/' \
		-e 's/TITLE/c-osc/' ) > c-osc.plot	
	
# DATAFILE, SAMPLERATE, DATALENGTH

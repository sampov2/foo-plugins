

all: c-osc.plot

clean:
	rm -f *.data *.plot *.octave

%.data: %.wav %.octave
	octave --silent $*.octave

%.octave: octave_template
	(echo 'basename = "$*";'; cat octave_template) > $*.octave


%.plot: gnuplot_template Makefile %.data
	#MAXSAMPLE=`(echo -n '20*l('; cat $*.data | sort -n | tail -1 | tr -d '\n '; echo ')/l(10)') | bc -l`
	#SAMPLERATE=`sndfile-info $*.wav | grep ^"Sample Rate" | cut -d ':' -f 2 | tr -d ' '`
	#DATALENGTH=`wc -l $*.data  | cut -d ' ' -f 1`

	(cat gnuplot_template | sed \
		-e 's/DATAFILE/$*.data/' \
		-e 's/SAMPLERATE/'`sndfile-info $*.wav | grep ^"Sample Rate" | cut -d ':' -f 2 | tr -d ' '`'/' \
		-e 's/DATALENGTH/'`wc -l $*.data  | cut -d ' ' -f 1`'/' \
		-e 's/MAXSAMPLE/'`(echo -n '20*l('; cat $*.data | sort -n | tail -1 | tr -d '\n '; echo ')/l(10)') | bc -l`'/' \
		-e 's/TITLE/$*/' ) > $@
	
# DATAFILE, SAMPLERATE, DATALENGTH

all: gschmooz-jack gschmooz-stereo-jack

gschmooz-jack: foo-comp-mono.dsp compressor-basics.dsp
	faust -svg -a jack-gtk.cpp foo-comp-mono.dsp > gen/foo-comp-mono.cpp
	g++ -O3 -mfpmath=sse -msse -msse2 -ffast-math gen/foo-comp-mono.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o gschmooz-jack

gschmooz-stereo-jack: foo-comp-stereo.dsp compressor-basics.dsp
	faust -svg -a jack-gtk.cpp foo-comp-stereo.dsp > gen/foo-comp-stereo.cpp
	g++ -O3 -mfpmath=sse -msse -msse2 -ffast-math gen/foo-comp-stereo.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o gschmooz-stereo-jack


gschmooz-sndfile: foo-comp-mono.dsp compressor-basics.dsp
	faust -svg -a sndfile.cpp foo-comp-mono.dsp > gen/gschmooz-sndfile.cpp
	g++ -O3 -mfpmath=sse -msse -msse2 -ffast-math gen/gschmooz-sndfile.cpp `pkg-config --cflags --libs sndfile` -o gschmooz-sndfile

clean:
	rm -fr gschmooz-jack gschmooz-stereo-jack foo-comp-mono.dsp-svg foo-comp-stereo.dsp-svg gen/*


PREFIX=/usr/local
CXX=g++
CFLAGS=-O3 -mfpmath=sse -msse -msse2 -ffast-math

CFLAGS_LV2=$(CFLAGS) -Wall -fPIC -DPIC `pkg-config --cflags lv2-plugin`
LDFLAGS_LV2=-shared -Wl,-Bdynamic `pkg-config --libs lv2-plugin`


all: lv2

lv2-full: minimal lv2	

lv2: lv2-manifest.ttl schmooz_mono_lv2.cpp schmooz_stereo_lv2.cpp 
	mkdir -p schmooz.lv2
	$(CXX) schmooz_mono_lv2.cpp   -o schmooz_mono_lv2.o   -c $(CFLAGS_LV2)
	$(CXX) schmooz_stereo_lv2.cpp -o schmooz_stereo_lv2.o -c $(CFLAGS_LV2)
	$(CXX) schmooz_mono_lv2.o schmooz_stereo_lv2.o -o schmooz.lv2/schmooz.so $(LDFLAGS_LV2)
	cp lv2-manifest.ttl schmooz.lv2/manifest.ttl
	
install: lv2
	cp -r schmooz.lv2 $(PREFIX)/lib/lv2/
	

# generates C++ classes from faust dsp code
minimal:
	faust -a minimal.cpp foo-comp-mono.dsp   > gen/foo-comp-mono-minimal.cpp
	faust -a minimal.cpp foo-comp-stereo.dsp > gen/foo-comp-stereo-minimal.cpp

# misc testers
gschmooz-jack: foo-comp-mono.dsp compressor-basics.dsp biquad-hpf.dsp
	faust -svg -a jack-gtk.cpp foo-comp-mono.dsp > gen/foo-comp-mono.cpp
	$(CXX) $(CFLAGS) gen/foo-comp-mono.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o gschmooz-jack

gschmooz-stereo-jack: foo-comp-stereo.dsp compressor-basics.dsp biquad-hpf.dsp
	faust -svg -a jack-gtk.cpp foo-comp-stereo.dsp > gen/foo-comp-stereo.cpp
	$(CXX) $(CFLAGS) -ffast-math gen/foo-comp-stereo.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o gschmooz-stereo-jack


gschmooz-sndfile: foo-comp-mono.dsp compressor-basics.dsp
	faust -svg -a sndfile.cpp foo-comp-mono.dsp > gen/gschmooz-sndfile.cpp
	$(CXX) $(CFLAGS) gen/gschmooz-sndfile.cpp `pkg-config --cflags --libs sndfile` -o gschmooz-sndfile

clean:
	rm -fr gschmooz-jack gschmooz-stereo-jack foo-comp-mono.dsp-svg foo-comp-stereo.dsp-svg gen/*

ladspaplugs: *.dsp
	faust -a ladspa.cpp foo-comp-mono.dsp \
		| sed 's/Label = .*;/Label = "Foo Schmooz Mono";/' \
		| sed 's/Name = .*;/Name = "Foo Schmooz Mono (alpha1)";/' \
		| sed 's/UniqueID = .*;/UniqueID = 3191;/' \
		| sed 's/Maker = .*;/Maker = "Sampo Savolainen";/' \
		> ladspa/foo-schmooz-mono.cpp
	$(CXX) -fPIC -shared $(CFLAGS) ladspa/foo-schmooz-mono.cpp -o ladspa/foo-schmooz-mono.so
	
	faust -a ladspa.cpp foo-comp-stereo.dsp \
		| sed 's/Label = .*;/Label = "Foo Schmooz Stereo";/' \
		| sed 's/Name = .*;/Name = "Foo Schmooz Stereo (alpha1)";/' \
		| sed 's/UniqueID = .*;/UniqueID = 3192;/' \
		| sed 's/Maker = .*;/Maker = "Sampo Savolainen";/' \
		> ladspa/foo-schmooz-stereo.cpp
	$(CXX) -fPIC -shared $(CFLAGS) ladspa/foo-schmooz-stereo.cpp -o ladspa/foo-schmooz-stereo.so


biquad-hpf-test: biquad-hpf.dsp
	faust -svg -a jack-gtk.cpp biquad-hpf.dsp > gen/biquad-hpf.cpp
	$(CXX) -O3 -mfpmath=sse -msse -msse2 -ffast-math gen/biquad-hpf.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o biquad-hpf-test

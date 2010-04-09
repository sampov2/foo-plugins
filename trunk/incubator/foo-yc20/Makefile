PREFIX=/usr/local
CXX=g++
#CFLAGS=-O3 -mfpmath=sse -msse -msse2 -ffast-math
#CFLAGS=-O3 -fgcse-sm -march=core2 -funsafe-math-optimizations -Wall

MYGCCFLAGS := -O3 -march=native -mfpmath=sse -msse -msse2 -msse3 -ffast-math -ftree-vectorize
#MYGCCFLAGS := -O3 -march=native -mfpmath=sse -msse -msse2 -msse3 -ffast-math -ftree-vectorize
VSIZE := 256

DEBUG_CFLAGS=-g

CFLAGS=-O3 -march=native -mfpmath=sse -msse -msse2 -msse3 -ffast-math -ftree-vectorize

CFLAGS2=-O3 -march=native -mfpmath=sse -msse -msse2 -msse3 -ffast-math -ftree-vectorize -fgcse-sm
CFLAGS3=-O3 -march=native -mfpmath=sse -msse -msse2 -msse3 -ffast-math -ftree-vectorize -fgcse-sm    -funsafe-math-optimizations


CFLAGS_LV2=$(CFLAGS) -Wall -fPIC -DPIC `pkg-config --cflags lv2-plugin`
LDFLAGS_LV2=-shared -Wl,-Bdynamic `pkg-config --libs lv2-plugin`

all: foo-yc20

# jackd -R -P30 -t5000 -dalsa -dhw:0 -r48000 -p128 -n3 -Xseq
# 37-39% DSP Load (CFLAGS #1)
# 35% DSP Load (CFLAGS #3)
#foo-yc20: *.dsp polyblep.cpp foo-yc20.cpp yc20_wdgts.h
#	faust -vec -vs 32 -a minimal.cpp yc20.dsp > gen/foo-yc20-minimal.cpp
#	$(CXX) -I. $(CFLAGS3) foo-yc20.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20

# jackd -R -P30 -t5000 -dalsa -dhw:0 -r48000 -p128 -n3 -Xseq
# 99% DSP Load
foo-yc20: *.dsp polyblep.cpp foo-yc20.cpp yc20_wdgts.h
	faust -sch -g -vs 256 -a minimal.cpp yc20.dsp > gen/foo-yc20-minimal.cpp
	$(CXX) -I. $(CFLAGS) foo-yc20.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20

# jackd -R -P30 -t5000 -dalsa -dhw:0 -r48000 -p128 -n3 -Xseq
# 48-50% DSP Load - but fast to compile unlike the others
#foo-yc20: *.dsp polyblep.cpp foo-yc20.cpp yc20_wdgts.h
#	rm -rf yc20-svg
#	faust -svg -a minimal.cpp yc20.dsp > gen/foo-yc20-minimal.cpp
#	$(CXX) -I. $(CFLAGS) foo-yc20.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20
# faust param			DSP %
#   nada			43
#   -vec			30
#   -vec -dfs			30

foo-yc20-jack: *.dsp jack-midi-gtk.cpp polyblep.cpp
	rm -rf yc20-svg
	faust -sch -svg -a jack-midi-gtk.cpp yc20.dsp > gen/yc20.cpp
	$(CXX) -I. $(CFLAGS) gen/yc20.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20-jack

foo-yc20gv-jack: *.dsp jack-midi-gtk.cpp polyblep.cpp
	faust -a jack-midi-gtk.cpp -vec -vs 32 yc20.dsp > gen/yc20gv.cpp
	$(CXX) -I. $(MYGCCFLAGS) gen/yc20gv.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20gv-jack

foo-yc20gp-jack: *.dsp jack-midi-gtk.cpp polyblep.cpp
	faust -a jack-midi-gtk.cpp -sch -g -vs 256 yc20.dsp > gen/yc20gp.cpp
	$(CXX) -I. $(MYGCCFLAGS) gen/yc20gp.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20gp-jack

yc20-plotter: *.dsp polyblep.cpp
	faust -a plot.cpp yc20.dsp > gen/yc20-plot.cpp
	$(CXX) -I. $(CFLAGS) gen/yc20-plot.cpp -o yc20-plotter
	echo './yc20-plotter -n 1000 > dump && bash cat dump  | sed "s/\t/\n/g" | grep -v ^$ > dump2 && gnuplot'

blep-plot: blep.dsp polyblep.cpp
	rm -rf blep-svg
	faust -svg -a plot.cpp blep.dsp > gen/blep-plot.cpp
	$(CXX) -I. $(CFLAGS) gen/blep-plot.cpp -o blep-plotter

blep-jack: blep.dsp polyblep.cpp
	rm -rf blep-svg
	faust -svg -a jack-gtk.cpp blep.dsp > gen/blep-jack.cpp
	$(CXX) -I. $(CFLAGS) gen/blep-jack.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o blep-jack

filtdesign: foo

filtdesign-jack: filtdesign.dsp 
	rm -rf filtdesign-svg
	faust -svg -a jack-console.cpp filtdesign.dsp > gen/filtdesign-jack.cpp
	$(CXX) -I. $(CFLAGS) -ljack gen/filtdesign-jack.cpp -o filtdesign-jack



filtdesign-plot: filtdesign.dsp 
	rm -rf filtdesign-svg
	faust -svg -a plot.cpp filtdesign.dsp > gen/filtdesign-plot.cpp
	$(CXX) -I. $(CFLAGS) gen/filtdesign-plot.cpp -o filtdesign-plotter


filtdesign-octave: filtdesign.dsp 
	rm -rf filtdesign-svg
	faust -svg -a octave.cpp filtdesign.dsp > gen/filtdesign-octave.cpp
	$(CXX) -I. $(CFLAGS) gen/filtdesign-octave.cpp -o filtdesign-octave


PREFIX=/usr/local
CXX=g++
#CFLAGS=-O3 -mfpmath=sse -msse -msse2 -ffast-math
CFLAGS=-O3 -fgcse-sm -march=core2 -funsafe-math-optimizations -Wall

CFLAGS_LV2=$(CFLAGS) -Wall -fPIC -DPIC `pkg-config --cflags lv2-plugin`
LDFLAGS_LV2=-shared -Wl,-Bdynamic `pkg-config --libs lv2-plugin`

all: foo-yc20-jack

foo-yc20-jack: *.dsp jack-midi-gtk.cpp
	rm -rf yc20-svg
	faust -svg -a jack-midi-gtk.cpp yc20.dsp > gen/yc20.cpp
	$(CXX) $(CFLAGS) gen/yc20.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20-jack

yc20-plotter: *.dsp
	faust -a plot.cpp yc20.dsp > gen/yc20-plot.cpp
	$(CXX) $(CFLAGS) gen/yc20-plot.cpp -o yc20-plotter
	echo './yc20-plotter -n 1000 > dump && bash cat dump  | sed "s/\t/\n/g" | grep -v ^$ > dump2 && gnuplot'

blep-plot: blep.dsp
	rm -rf blep-svg
	faust -svg -a plot.cpp blep.dsp > gen/blep-plot.cpp
	$(CXX) -I. $(CFLAGS) gen/blep-plot.cpp -o blep-plotter

blep-jack: blep.dsp
	rm -rf blep-svg
	faust -svg -a jack-gtk.cpp blep.dsp > gen/blep-jack.cpp
	$(CXX) $(CFLAGS) gen/blep-jack.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o blep-jack



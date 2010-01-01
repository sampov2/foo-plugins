PREFIX=/usr/local
CXX=g++
CFLAGS=-O3 -mfpmath=sse -msse -msse2 -ffast-math

CFLAGS_LV2=$(CFLAGS) -Wall -fPIC -DPIC `pkg-config --cflags lv2-plugin`
LDFLAGS_LV2=-shared -Wl,-Bdynamic `pkg-config --libs lv2-plugin`

all: foo-yc20-jack

foo-yc20-jack: *.dsp
	rm -rf yc20-svg
	faust -svg -a jack-gtk.cpp yc20.dsp > gen/yc20.cpp
	$(CXX) $(CFLAGS) gen/yc20.cpp `pkg-config --cflags --libs gtkmm-2.4 jack` -o foo-yc20-jack


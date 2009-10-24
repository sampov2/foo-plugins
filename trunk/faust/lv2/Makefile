CXX=/usr/bin/g++

CFLAGS=-O2 -Wall -ffast-math -mfpmath=sse -msse -msse2 -fPIC -DPIC `pkg-config --cflags lv2-plugin`
LDFLAGS=-shared -Wl,-Bdynamic `pkg-config --libs lv2-plugin`

all: build-schmooz-lv2

build-schmooz-lv2: manifest.ttl schmooz.cpp
	$(CXX) schmooz.cpp -o schmooz.o -c $(CFLAGS)
	$(CXX) schmooz.o -o schmooz.lv2/schmooz.so $(LDFLAGS)
	cp manifest.ttl schmooz.lv2/

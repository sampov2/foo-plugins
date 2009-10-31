CC=gcc
CFLAGS=-O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -fPIC -DPIC -g -msse -mfpmath=sse
#CFLAGS=-g -fPIC -DPIC

LIBFLAGS=-nostartfiles -shared -Wl,-Bsymbolic

PLUGIN_LIBS    = foo_limiter.so foo_limiter_v2.so foo_transients.so foo_transients_mono.so foo_driver.so t00b_limiter.so foo_saturator.so foo_chop.so foo_transients_v2.so
PLUGIN_SOURCES = foo_limiter.c  foo_limiter_v2.c  foo_transients.c  foo_transients_mono.c  foo_driver.so t00b_limiter.c foo_saturator.c foo_chop.c foo_transients_v2.c

all: plugins tester

%.so : %.o rms.o
	$(CC) $(LIBFLAGS) rms.o $< -o $@ -lm

%.c : %.xml makestub.pl
	./makestub.pl $< > $@


plugins: $(PLUGIN_LIBS)

tester.o: foo_limiter.xml foo_transients.xml t00b_limiter.xml

tester: tester.o rms.o $(PLUGIN_SOURCES)
	gcc tester.o rms.o -o tester -lm 

tester.o: $(PLUGIN_SOURCES)

$(PLUGIN_SOURCES): utils.h rms.h

rms.o: rms.h

install: plugins
	cp $(PLUGIN_LIBS) /usr/lib/ladspa/

clean:
	rm -f *.o *.so $(PLUGIN_SOURCES) tester plot/*


#

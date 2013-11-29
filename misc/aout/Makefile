#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= -pipe $(ACC_WARNINGS) 

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)


all: aout malloc test MANIFEST

malloc: malloc.o
	$(LINK.c) -o malloc malloc.o

test: test.o
	$(LINK.c) -o test test.o

aout: aout.o
	$(LINK.c) -o aout aout.o

clean:
	rm -f malloc.o aout.o

realclean: clean
	rm -f malloc aout

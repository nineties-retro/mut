#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= -pipe $(ACC_WARNINGS)

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)


all: trace debug

trace: trace.o
	$(LINK.c) -o trace trace.o

debug: debug.o
	$(LINK.c) -o debug debug.o

clean:
	rm -f debug.o trace.o

realclean: clean
	rm -f debug trace

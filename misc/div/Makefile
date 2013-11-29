#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= -pipe $(ACC_WARNINGS) -O4 -m486 -fomit-frame-pointer

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)

exes= div_8_10 div_16_10 div_32_16 div_64_16

all: $(exes)

div_8_10: div_8_10.o
	$(LINK.c) -o div_8_10 div_8_10.o

div_16_10: div_16_10.o
	$(LINK.c) -o div_16_10 div_16_10.o

div_32_16: div_32_16.o
	$(LINK.c) -o div_32_16 div_32_16.o

div_64_16: div_64_16.o
	$(LINK.c) -o div_64_16 div_64_16.o

%.o:	%.c
	$(COMPILE.c) -c -o $@ $<

clean:
	rm -f $(exes:%=%.o)

realclean: clean
	rm -f $(exes)

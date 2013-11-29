#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= -pipe $(ACC_WARNINGS) -g

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)


all: bar elf MANIFEST 

elf: elf.o
	$(LINK.c) -o elf elf.o

bar: foo.o bar.o
	$(LINK.c) -o bar foo.o bar.o

clean:
	rm -f elf.o

realclean: clean
	rm -f elf

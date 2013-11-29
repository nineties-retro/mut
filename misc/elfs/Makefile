#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= -pipe $(ACC_WARNINGS) 

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)


all: elf

elf: elf.o
	$(LINK.c) -o elf elf.o

clean:
	rm -f elf.o

realclean: clean
	rm -f elf

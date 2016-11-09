#.intro: GNU Makefile

CFLAGS = -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings -g
all: elf

elf: elf.o
	$(LINK.c) -o elf elf.o

clean:
	rm -f elf.o

realclean: clean
	rm -f elf

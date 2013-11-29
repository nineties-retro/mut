#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= $(ACC_WARNINGS)

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)

AS=gas

all: ptrace regs

ptrace: ptrace.o
	$(LINK.c) -o ptrace ptrace.o

regs: regs.o
	$(LINK.c) -o regs regs.o

clean:
	rm -f  ptrace.o

realclean: clean
	rm -f ptrace

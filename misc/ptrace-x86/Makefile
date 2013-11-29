#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= -pipe $(ACC_WARNINGS)

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)


all: breakpoint ptrace

breakpoint: breakpoint.o
	$(LINK.c) -o breakpoint breakpoint.o

ptrace: ptrace.o
	$(LINK.c) -o ptrace ptrace.o

manifest= Makefile README breakpoint.c ptrace.c

clean:
	rm -f breakpoint.o ptrace.o

realclean: clean
	rm -f breakpoint ptrace

#.intro: GNU Makefile

ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
ACC_CFLAGS= -pipe $(ACC_WARNINGS)

ACC=gcc

COMPILE.c= $(ACC) $(ACC_CFLAGS) $(ACC_CPPFLAGS) -c

LINK.c= $(ACC) $(LDFLAGS)

tests= cpu_limit data_limit

all: $(tests)

cpu_limit: cpu_limit.o
	$(LINK.c) -o cpu_limit cpu_limit.o

data_limit: data_limit.o
	$(LINK.c) -o data_limit data_limit.o

clean:
	rm -f $(tests:%=%.o)

realclean: clean
	rm -f $(tests)

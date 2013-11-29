#.intro: GNU Makefile

all:
	make -C src all

clean:
	make -C src clean

distclean:
	find . -name '*~' -print0 | xargs -0 rm -f

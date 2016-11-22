#.intro: GNU Makefile

all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

distclean:
	find . -name '*~' -print0 | xargs -0 rm -f

#.intro: GNU Makefile

roff= groff -Tascii -man

.PHONEY: all

all:	mut.doc mut.txt


mut.doc:	mut.1
	$(roff) mut.1 > $@

mut.txt:	mut.doc
	col -b < mut.doc > $@

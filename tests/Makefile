#.intro: GNU Makefile
#
# See <URI:rld/tests/README#compiling.flags>
arch=x86
CC=gcc
fp=fp
opt=off
linkage=dynamic
#exec=aout
exec=elfd
debug=on

DEPCC=gcc
ACC_WARNINGS= -Wall -Wcast-align -Wcast-qual -Winline \
              -Wmissing-prototypes -Wpointer-arith \
              -Wstrict-prototypes -Wwrite-strings
CFLAGS = $(FRAME_POINTER) $(ACC_OPT) $(ACC_DEBUG) $(ACC_WARNINGS)
LDFLAGS = $(LINKAGE)
LINK.c = $(CC) $(LDFLAGS)
COMPILE.c = $(CC) $(CFLAGS) -c 

VARIANT= gen/$(CC).$(arch).$(fp).$(linkage).$(exec).$(opt).$(debug)

#.tests:
tests=	calloc callocn callocv \
	malloc0 malloc1 mallocn \
	realloc reallocn \
	count64 space_count \
	ffm fum_free fum_realloc fum_free_chain \
	mlk_malloc mlk_realloc0 mlk_realloc1 segv \
	afm_free not_func incest wum_calloc wum_malloc

all:	$(VARIANT)/obj \
        $(VARIANT)/exe \
	$(tests:%=$(VARIANT)/exe/%.exe) \
	$(tests:%=$(VARIANT)/exe/%.exs) \

$(VARIANT)/exe/%.exe: $(VARIANT)/obj/%.o
	@echo Linking $* ...
	@$(LINK.c) -o $@ $<

# Aargh, Solaris strip does not take -o, need to think of something

$(VARIANT)/exe/%.exs: $(VARIANT)/exe/%.exe
	@echo stripping $*
	@strip -o $@ $<

manifest= Makefile README make gen src $(tests:%=src/%.c)

include make/debug.$(debug)
include make/cc.$(CC).fp.$(fp)
include make/cc.$(CC).link.$(linkage)
include make/cc.$(CC).opt.$(opt)

$(VARIANT)/obj/%.o: src/%.c
	@echo $(CC) $< ...
	@$(COMPILE.c) $< -o $@

$(VARIANT):
	@test -d $@ || mkdir -p $@

$(VARIANT)/obj: $(VARIANT)
	@test -d $@ || mkdir $@

$(VARIANT)/exe: $(VARIANT)
	@test -d $@ || mkdir $@

clean:
	rm -rf $(tests:%=$(VARIANT)/%.exe)

distclean:	clean
	rm -rf $(VARIANT)/depend

realclean:
	rm -rf gen

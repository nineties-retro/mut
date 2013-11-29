# $Id$
#
#.intro: src GNU Makefile
#
#.stable
# See <URI:mut/src/README#compiling.flags.stable>
mut_opt_assert=vendor
mut_opt_backtrace=fixed
mut_opt_depend_platform=unix
mut_opt_elf=vendor
mut_opt_errno=vendor
mut_opt_obj_ext=o
mut_opt_ptrace=vendor
mut_opt_stddef=vendor
mut_opt_stdio=vendor
mut_opt_stdlib=vendor
mut_opt_string=vendor
mut_opt_resources=list
mut_opt_ui=tty

#.volatile
# See <URI:mut/src/README#compiling.flags.volatile>
mut_opt_arch=x86
mut_opt_cc=gcc
mut_opt_counter=uint64
mut_opt_exec=elf
mut_opt_debug=on
mut_opt_log=tty
mut_opt_mem=debug
mut_opt_opt=off
mut_opt_os=linux
mut_opt_proc=ptrace
mut_opt_uint64=struct
mut_opt_warn=on

mut_build_root=.
mut_out_root=out
mut_bin_root=$(mut_build_root)/bin
mut_etc_root=$(mut_build_root)/etc

.PHONY: all clean

all:	mut_tty
clean:	mut_tty_realclean

mut_cc_depend := $(mut_bin_root)/depend_$(mut_opt_depend_platform).sh

mut_cc_includes = $(mut_$(mut_target)_dirs:%=-I%)

mut_cc_gcc_out := -o 
mut_cc_gcc_debug_on := -g
mut_cc_gcc_debug_off := 
mut_cc_gcc_opt_on := -O3
mut_cc_gcc_opt_off := 
mut_cc_gcc_warn_on := -Wall -Wcast-align -Wcast-qual -Winline \
                       -Wmissing-prototypes -Wpointer-arith \
                       -Wstrict-prototypes -Wwrite-strings
mut_cc_gcc_warn_off :=
mut_cc_gcc_misc := 

#
# Note that $(mut_cc_$(mut_opt_cc)_out) is deliberately the last
# item in the list so that the /Fo option works correctly for cl.
#
mut_compile_c = $(mut_opt_cc) $(mut_cc_$(mut_opt_cc)_misc) \
		 $(mut_cc_$(mut_opt_cc)_debug_$(mut_opt_debug)) \
		 $(mut_cc_$(mut_opt_cc)_opt_$(mut_opt_opt)) \
		 $(mut_cc_$(mut_opt_cc)_warn_$(mut_warn)) \
		 $(mut_cc_includes) \
                 -c $(mut_cc_$(mut_opt_cc)_out)

define mut_compile_c_action
  @echo $(mut_opt_cc) $< ...
  @test -d $(dir $@) || mkdir -p $(dir $@)
  @$(mut_compile_c)$@ $<
endef


mut_link_c := $(mut_opt_cc)

define mut_link_c_action
  @echo Linking $(notdir $@) ...
  @$(mut_link_c) -o $@ $(mut_$(mut_target)_obj)
endef

empty:=
space:= $(empty) $(empty)

mut_tty_variant= tty.$(mut_opt_arch).$(mut_opt_cc).$(mut_opt_counter).$(mut_opt_exec).$(mut_opt_debug).$(mut_opt_log).$(mut_opt_mem).$(mut_opt_opt).$(mut_opt_os).$(mut_opt_proc).$(mut_opt_uint64).$(mut_opt_warn)

mut_tty_out = $(mut_out_root)/$(mut_tty_variant)

mut_tty_src := \
	mem/$(mut_opt_mem)/mut_mem.c \
	uint64/$(mut_opt_uint64)/mut_uint64.c \
	log/$(mut_opt_log)/mut_log.c \
	counter/$(mut_opt_counter)/mut_counter.c \
	exec/mut_exec_symtab.c \
	exec/$(mut_opt_exec)/mut_exec.c \
        backtrace/$(mut_opt_backtrace)/mut_backtrace.c \
	proc/$(mut_opt_proc)/arch/$(mut_opt_arch)/os/$(mut_opt_os)/mut_trace_arch.c \
	proc/$(mut_opt_proc)/arch/$(mut_opt_arch)/mut_process_arch.c \
	proc/$(mut_opt_proc)/mut_trace.c \
	proc/$(mut_opt_proc)/mut_process.c \
	resources/$(mut_opt_resources)/mut_resources.c \
	mut.c \
	ui/$(mut_opt_ui)/mut_ui.c

mut_tty_dirs := \
	arch/$(mut_opt_arch) \
	include/assert/$(mut_opt_assert) \
	include/errno/$(mut_opt_errno) \
	include/stddef/$(mut_opt_stddef) \
	include/stdio/$(mut_opt_stdio) \
	include/stdlib/$(mut_opt_stdlib) \
	include/string/$(mut_opt_string) \
	mem/$(mut_opt_mem) \
	uint64/$(mut_opt_uint64) \
	log/$(mut_opt_log) \
	counter/$(mut_opt_counter) \
	exec/$(mut_opt_exec)/arch/$(mut_opt_arch) \
	exec/$(mut_opt_exec)/elf/$(mut_opt_elf) \
	exec/$(mut_opt_exec) \
	exec \
        backtrace/$(mut_opt_backtrace) \
	proc/$(mut_opt_proc)/arch/$(mut_opt_arch)/os/$(mut_opt_os) \
	proc/$(mut_opt_proc)/arch/$(mut_opt_arch) \
	proc/$(mut_opt_proc)/ptrace/$(mut_opt_ptrace) \
	proc/$(mut_opt_proc) \
	resources/$(mut_opt_resources) \
	ui/$(mut_opt_ui) \
	.


mut_tty_deps := $(mut_tty_src:%.c=$(mut_tty_out)/depend/%.d)
mut_tty_obj := $(mut_tty_src:%.c=$(mut_tty_out)/obj/%.$(mut_opt_obj_ext))


-include $(mut_tty_deps)

#
# The spacing of $(mut_compile_c)$@ is deliberate so that the /Fo option
# works in cl.
#
$(mut_tty_out)/obj/%.$(mut_opt_obj_ext): %.c
	$(mut_compile_c_action)

$(mut_tty_out)/depend/%.d: %.c
	@echo mkdepend $< ...
	@test -d $(@D) || mkdir -p $(@D)
	@$(mut_cc_depend) $(mut_bin_root) $< $@ $(mut_tty_out)/obj/$(basename $<).$(mut_opt_obj_ext) $(subst $(space),:,$(mut_tty_dirs))


mut_tty: $(mut_tty_out)/bin/mut_tty
$(mut_tty_out)/bin/mut_tty: mut_target = tty
$(mut_tty_out)/bin/mut_tty: \
		$(mut_tty_out)/bin \
                $(mut_tty_out)/obj \
                $(mut_tty_obj) \
                $(mut_tty_out)/depend \
                $(mut_tty_deps)
	$(mut_link_c_action)

$(mut_tty_out)/bin \
$(mut_tty_out)/depend \
$(mut_tty_out)/obj:
	@test -d $@ || mkdir -p $@


mut_tty_clean:
	rm -rf $(mut_tty_out)/obj

mut_tty_distclean:	mut_tty_clean
	rm -rf $(mut_tty_out)/depend

mut_tty_realclean:
	rm -rf $(mut_tty_out)

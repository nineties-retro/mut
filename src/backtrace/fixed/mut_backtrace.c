/*
 * $Id$
 */

#include "mut_assert.h"		/* mut_assert_check */
#include "mut_stddef.h"		/* size_t */
#include "mut_mem.h"		/* mut_mem_malloc, mut_mem_free */
#include "mut_errno.h"		/* errno */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log.h"
#include "mut_log_exec_addr.h"
#include "mut_exec.h"
#include "mut_backtrace_open.h"
#include "mut_backtrace_close.h"
#include "mut_backtrace_enter.h"
#include "mut_backtrace_sharer.h"
#include "mut_log_backtrace.h"

mut_backtrace *mut_backtrace_open(mut_log *log, mut_exec * exec)
{
	mut_backtrace * bt= mut_mem_malloc(sizeof(mut_backtrace));
	if (bt == (mut_backtrace *)0) {
		mut_log_mem_full(log, errno);
	} else {
		bt->ref_count= 1;
		bt->n_frames= 0;
		bt->exec= exec;
	}
	return bt;
}


int mut_backtrace_enter(mut_backtrace *bt, mut_exec_addr ra)
{
	mut_assert_pre(bt != (mut_backtrace *)0);
	if (bt->n_frames < mut_backtrace_frames_max) {
		bt->frame[bt->n_frames].return_addr= ra;
		bt->n_frames += 1;
		return 1;
	} else {
		return -1;
	}
}


mut_backtrace *mut_backtrace_share(mut_backtrace *bt)
{
	mut_assert_pre(bt != (mut_backtrace *)0);
	bt->ref_count += 1;
	return bt;
}


void mut_log_backtrace(mut_log *log, mut_backtrace *bt)
{
	size_t i;
	mut_assert_pre(bt != (mut_backtrace *)0);
	mut_log_uint(log, bt->n_frames);
	for (i= 0; i < bt->n_frames; i+=1) {
		mut_exec_addr addr= bt->frame[i].return_addr;
		char const *name;
		mut_exec_addr function_addr;
		char const *file_name;
		unsigned int line_number;
		int status= mut_exec_addr_name(bt->exec, addr, &function_addr,
					       &name, &file_name, &line_number);
		if (status) {
#if XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			mut_assert_check(function_addr - addr);
#endif
			mut_log_string(log, name);
			mut_log_exec_addr(log, function_addr);
			mut_log_exec_addr(log, addr);
			mut_log_uint(log, addr - function_addr);
			mut_log_string(log, (file_name == 0) ? "?" : file_name);
			mut_log_uint(log, line_number);
		} else {
			mut_log_string(log, "?");
			mut_log_exec_addr(log, addr);
		}
	}
}



void mut_backtrace_close(mut_backtrace *bt)
{
	mut_assert_pre(bt != (mut_backtrace *)0);
	mut_assert_pre(bt->ref_count != 0);
	if (--bt->ref_count == 0)
		mut_mem_free(bt);
}

/*
 *.intro: the core of MUT.
 *
 */

#include "mut_assert.h"		/* mut_assert_check, mut_assert_pre */
#include "mut_stddef.h"		/* size_t */
#include "mut_log.h"		/* mut_log */
#include "mut_errno.h"		/* errno */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_instr.h"		/* mut_instr */
#include "mut_mem.h"		/* mut_mem_malloc */
#include "mut_arg.h"		/* mut_arg */
#include "mut.h"
#include "mut_backtrace_open.h" /* mut_backtrace, mut_backtrace_open */
#include "mut_backtrace_close.h" /* mut_backtrace_close */
#include "mut_backtrace_sharer.h" /* mut_backtrace_share */
#include "mut_log_backtrace.h"	/* mut_log_backtrace */
#include "mut_process.h"
#include "mut_log_counter.h"	/* mut_log_counter */


/*
 * `mut_actions_enter' enters `action' into the address -> action map.
 * If an action with the same address is already registered in the
 * action map, then that action is returned.  If there is no such action
 * then `action' is returned.  It is assumed that no two actions have the
 * same address and name and so it should be possible to tell the two
 * actions apart based on their name.
 */
static mut_action *mut_actions_enter(mut *m, mut_action *action)
{
	mut_action *a = m->actions.root;

	for (; a != (mut_action *)0; a = a->next) {
		if (a->addr == action->addr)
			return a;
	}
	action->next = m->actions.root;
	m->actions.root = action;
	return action;
}



/*
 * `mut_actions_lookup' looks for an action with the given `action_addr'.
 * If an action with the same address is registered in the action map
 * then that action is returned.  If there is no such action then
 * (mut_action *)0 is returned.
 */
static mut_action *mut_actions_lookup(mut *m, mut_exec_addr action_addr)
{
	mut_action * action = m->actions.root;
loop:
	if (action == (mut_action *)0) {
		return action;
	} else if (action->addr == action_addr) {
		return action;
	} else {
		action = action->next;
		goto loop;
	}
}



static int
mut_manage_action(mut *m, mut_action *a,
		  struct mut_exec_function *f)
{
	mut_action *oa;

	a->hits = mut_counter_from_uint(0);
	if (f->flags == 0)
		return 1;
	a->addr = f->addr;
	oa = mut_actions_enter(m, a);
	if (oa->name != a->name) {
		mut_log_info(m->log, "duplicate.addr");
		mut_log_string(m->log, a->name);
		mut_log_string(m->log, oa->name);
		mut_log_exec_addr(m->log, a->addr);
		(void)mut_log_end(m->log);
		return 1;
	} 
	return mut_process_breakpoint(&m->process, a->addr, &a->instr);
}


#define mut_n_routines 6

static int mut_manage_actions(mut *m, const char *usage_function)
{
	struct mut_exec_function function[mut_n_routines];
	size_t n = (usage_function == 0) ? mut_n_routines - 1 : mut_n_routines;
	function[0].name = m->actions.malloc.name =  "malloc";
	function[1].name = m->actions.calloc.name =  "calloc";
	function[2].name = m->actions.realloc.name = "realloc";
	function[3].name = m->actions.free.name =    "free";
	function[4].name = m->actions.cfree.name =   "cfree";
	function[5].name = m->actions.usage.name =   usage_function;
     
	mut_exec_functions_addr(&m->exec, n, function);
	m->actions.malloc.type = mut_action_type_malloc;
	if (!mut_manage_action(m, &m->actions.malloc, &function[0]))
		return 0;
	m->actions.calloc.type = mut_action_type_calloc;
	if (!mut_manage_action(m, &m->actions.calloc, &function[1]))
		return 0;
	m->actions.realloc.type = mut_action_type_realloc;
	if (!mut_manage_action(m, &m->actions.realloc, &function[2]))
		return 0;
	m->actions.free.type = mut_action_type_consumer;
	if (!mut_manage_action(m, &m->actions.free, &function[3]))
		return 0;
	m->actions.cfree.type = mut_action_type_consumer;
	if (!mut_manage_action(m, &m->actions.cfree, &function[4]))
		return 0;
	if (usage_function != 0) {
		m->actions.usage.type = mut_action_type_usage;
		if (!mut_manage_action(m, &m->actions.usage, &function[5]))
			return 0;
	}
	return 1;
}



/*
 * `mut_open' opens `m' for use.
 *
 * `argv' is the command line for the command (including the command name) 
 * to be executed under the control of `m'.
 *
 * `mut_open' returns a non-zero value on success, zero if a fatal
 * error occurs.
 */
int mut_open(mut *m, char **argv, mut_log *l, mut_control flags,
	     char const *undesirable_suffix, const char *usage_function)
{
	if (!mut_exec_open(&m->exec, argv[0], l, undesirable_suffix))
		goto could_not_open_exec;
	if (!mut_process_open(&m->process, argv, l))
		goto could_not_open_process;
	m->log = l;
	m->flags = flags;
	if ((m->flags & mut_control_backtrace)
	    &&  !mut_exec_has_backtrace_symbols(&m->exec)) {
		m->flags &= ~mut_control_backtrace;
		mut_log_info(m->log, "backtrace.off");
		(void)mut_log_end(m->log);
	}
	m->stats.now.space = mut_counter_from_uint(0);
	m->stats.now.count = mut_counter_from_uint(0);
	m->stats.total.space = mut_counter_from_uint(0);
	m->stats.total.count = mut_counter_from_uint(0);
	m->stats.max.space.space = mut_counter_from_uint(0);
	m->stats.max.space.count = mut_counter_from_uint(0);
	m->stats.max.count.space = mut_counter_from_uint(0);
	m->stats.max.count.count = mut_counter_from_uint(0);

	m->actions.root = (mut_action *)0;
	if (!mut_manage_actions(m, usage_function))
		goto could_not_manage;
	mut_resources_open(&m->resources, l, flags & mut_control_free);
	return 1;

could_not_manage:
	mut_process_close(&m->process);
could_not_open_process:
	(void)mut_exec_close(&m->exec);
could_not_open_exec:
	return 0;
}


#define mut_zone_size sizeof(int)
#define mut_zone_value 0xDEADBEEF


/*
 * Round up a size so that it includes the space for the safety zone.
 * To ensure the zones are aligned, the object size is rounded up to
 * the nearest word.
 */
static size_t mut_round_up(size_t size)
{
	return ((size+mut_zone_size-1)&~(mut_zone_size-1))+2*mut_zone_size;
}


/*
 * Performing an action consists of :-
 *
 *.set_bp: setting a breakpoint at `return_addr' of `action'.
 *.restart: restarting the process.
 *.waiting: Waiting for the process to stop.
 *.check: if all went well, and assuming that the action doesn't (indirectly) 
 *  call itself or another action, then the process should be stopped
 *  on the breakpoint set in .set_bp.
 *.restore: restore the instruction that was overwritten in .set_bp.
 */
static int mut_do_action(mut *m, mut_exec_addr return_addr, mut_action *action)
{
	mut_exec_addr return_pc;
	mut_instr return_instr;

	if (!mut_process_breakpoint(&m->process, return_addr, &return_instr))
		return 0;
	if (!mut_process_resume(&m->process))
		return 0;
	if (mut_process_wait(&m->process, &return_pc) <= 0)
		return 0;
	if (return_pc != return_addr) {
		mut_log_fatal(m->log, "action.addr", 0);
		mut_log_string(m->log, action->name);
		mut_log_exec_addr(m->log, return_pc);
		mut_log_exec_addr(m->log, return_addr);
		(void)mut_log_end(m->log);
		return 0;
	}
	return mut_process_restore(&m->process, return_addr, return_instr);
}



static void mut_report_usage(mut *m)
{
	mut_resource *r;
	mut_resources_iter iter;
	size_t n_obj;

	mut_log_info(m->log, "mem.usage");
	n_obj = mut_resources_iter_open(&m->resources, &iter, 0);
	mut_log_ulong(m->log, (unsigned long)n_obj);
	while ((r = mut_resources_iter_read(&iter)) != (mut_resource *)0) {
		mut_log_string(m->log, mut_resource_producer_name(r));
		mut_log_exec_addr(m->log, r->addr);
		mut_log_ulong(m->log, (unsigned long)r->size);
		if (m->flags & mut_control_backtrace)
			mut_log_backtrace(m->log, r->producer.backtrace);
	}
	mut_resources_iter_close(&iter);
	(void)mut_log_end(m->log);
}




/*
 * `mut_collect_producer_stats' collects various statistics about a
 * producer call.  This includes he current space allocated, the number of
 * objects allocated ... etc.
 */
static void mut_collect_producer_stats(mut *m, size_t size)
{
	m->stats.now.space = mut_counter_inc(m->stats.now.space, size);
	m->stats.now.count = mut_counter_inc(m->stats.now.count, 1);
	m->stats.total.space = mut_counter_inc(m->stats.total.space, size);
	m->stats.total.count = mut_counter_inc(m->stats.total.count, 1);
	if (mut_counter_lt(m->stats.max.space.space, m->stats.now.space)) {
		m->stats.max.space.space = m->stats.now.space;
		m->stats.max.space.count = m->stats.now.count;
	}
	if (mut_counter_lt(m->stats.max.count.count, m->stats.now.count)) {
		m->stats.max.count.space = m->stats.now.space;
		m->stats.max.count.count = m->stats.now.count;
	}
}



/*
 * `mut_malloc_failure' is called when malloc returns 0.  It 
 * simply logs a warning.  If tracing is enabled, a trace is
 * is also logged, optionally with a backtrace.
 *
 * `mut_malloc_failure' is passed the ownership of `bt' which means it
 * is up to `mut_malloc_failure' to close `bt' if it is no longer needed.
 */
static void mut_malloc_failure(mut *m, size_t size, mut_backtrace *bt)
{
	mut_action *malloc = &m->actions.malloc;
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, malloc->name);
		mut_log_uint(m->log, (unsigned long)size);
		mut_log_exec_addr(m->log, 0);
		if (m->flags & mut_control_backtrace) 
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	mut_log_warning(m->log, "MR0");
	mut_log_string(m->log, malloc->name);
	mut_log_uint(m->log, (unsigned long)size);
	if (m->flags & mut_control_backtrace) 
		mut_log_backtrace(m->log, bt);
	(void)mut_log_end(m->log);
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
}



/*
 * `mut_malloc_success' is called when malloc returns a non-zero value.
 * `mut_malloc_success' creates a resource for the new object and
 * marks it as created by malloc.  If tracing is enabled a trace is
 * logged, optionally including a backtrace.
 *
 * `mut_malloc_success' returns a non-zero value if all goes well
 * and zero if not.
 */
static int mut_malloc_success(mut *m, size_t size, mut_exec_addr obj_addr,
			      mut_backtrace *bt)
{
	mut_action   *malloc = &m->actions.malloc;
	mut_resource *obj;

	mut_assert_pre((m->flags & mut_control_backtrace) || (bt == 0));

	if (m->flags & mut_control_stats)
		mut_collect_producer_stats(m, size);

	obj = mut_resources_create_resource(&m->resources, obj_addr);
	if (obj == (mut_resource *)0)
		return 0;

	if (!mut_resource_is_new(obj)) {
		mut_log_error(m->log, "MMM");
		mut_log_string(m->log, malloc->name);
		mut_log_string(m->log, mut_resource_producer_name(obj));
		mut_log_exec_addr(m->log, obj_addr);
		(void)mut_log_end(m->log);
		return 0;
	} 
	mut_resource_make_producer(obj, malloc, size, bt);
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, malloc->name);
		mut_log_uint(m->log, size);
		mut_log_exec_addr(m->log, obj_addr);
		if (m->flags & mut_control_backtrace) 
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	if (m->flags & mut_control_usage)
		mut_report_usage(m);
	return 1;
}



static int mut_produce_backtrace(mut *m, mut_backtrace **bt)
{
	if (m->flags & mut_control_backtrace) {
		if ((*bt = mut_backtrace_open(m->log, &m->exec)) == 0)
			return 0;
		if (!mut_process_function_backtrace(&m->process, *bt)) {
			mut_backtrace_close(*bt);
			return 0;
		}
	} else {
		*bt = (mut_backtrace *)0;
	}
	return 1;
}



static int mut_install_check_zones(mut *m, mut_exec_addr *obj_addr, size_t size)
{
	size_t pre_zone_addr = *obj_addr;
	size_t post_zone_addr = *obj_addr+size-mut_zone_size;

	*obj_addr += mut_zone_size;
	if (!mut_process_write_word(&m->process, pre_zone_addr, mut_zone_value))
		return 0;
	if (!mut_process_write_word(&m->process, post_zone_addr, mut_zone_value))
		return 0;
	if (!mut_process_write_result(&m->process, *obj_addr))
		return 0;
	return 1;
}



/*
 * `mut_do_malloc' ...
 *
*/
static int mut_do_malloc(mut *m, mut_exec_addr pc, mut_action *malloc)
{
	mut_arg         size;
	mut_exec_addr   return_addr;
	mut_arg         obj_addr_arg;
	mut_exec_addr   obj_addr;
	mut_backtrace * bt;
	size_t          adjusted_size;

	if (!mut_process_read_arg(&m->process, 0, &size))
		return 0;
	if (!mut_produce_backtrace(m, &bt))
		return 0;
	if (!mut_process_function_return_addr(&m->process, &return_addr))
		goto free_backtrace;
	if (m->flags & mut_control_check) {
		adjusted_size = mut_round_up(size);
		if (!mut_process_write_arg(&m->process, 0, adjusted_size))
			goto free_backtrace;
	}
	if (!mut_do_action(m, return_addr, malloc))
		goto free_backtrace;
	if (!mut_process_read_result(&m->process, &obj_addr_arg))
		goto free_backtrace;
	if (m->flags & mut_control_stats)
		malloc->hits = mut_counter_inc(malloc->hits, 1);
	if (obj_addr_arg == 0) {
		mut_malloc_failure(m, size, bt);
		return 1;
	} 
	obj_addr = mut_exec_addr_from_int(obj_addr_arg);
	if ((m->flags & mut_control_check)
	    &&  !mut_install_check_zones(m, &obj_addr, adjusted_size))
		goto free_backtrace;
	return mut_malloc_success(m, size, obj_addr, bt);

free_backtrace:
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	return 0;
}



/*
 * `mut_calloc_failure' is called when calloc returns 0.  It 
 * simply logs a warning.  If tracing is enabled, a trace is
 * is also logged, optionally with a backtrace.
 *
 * `mut_calloc_failure' returns a non-zero value if all goes well
 * and zero if not.
 */
static void mut_calloc_failure(mut *m, size_t nelems, size_t elem_size, mut_backtrace *bt)
{
	mut_action *calloc = &m->actions.calloc;

	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, calloc->name);
		mut_log_uint(m->log, (unsigned long)nelems);
		mut_log_uint(m->log, (unsigned long)elem_size);
		mut_log_exec_addr(m->log, 0);
		if (m->flags & mut_control_backtrace) 
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	mut_log_warning(m->log, "CR0");
	mut_log_string(m->log, calloc->name);
	mut_log_uint(m->log, (unsigned long)nelems);
	mut_log_uint(m->log, (unsigned long)elem_size);
	if (m->flags & mut_control_backtrace) 
		mut_log_backtrace(m->log, bt);
	(void)mut_log_end(m->log);
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
}



/*
 * `mut_calloc_success' is called when calloc returns a non-zero value.
 * `mut_calloc_success' creates a resource for the new object and
 * marks it as created by calloc.  If tracing is enabled a trace is
 * logged, optionally including a backtrace.
 *
 * `mut_calloc_success' returns a non-zero value if all goes well
 * and zero if not.
 */
static int mut_calloc_success(mut *m, size_t n_elems, size_t elem_size,
			      mut_exec_addr obj_addr, mut_backtrace *bt)
{
	mut_action *calloc = &m->actions.calloc;
	mut_resource *obj;
	size_t size = n_elems*elem_size;

	if (m->flags & mut_control_stats)
		mut_collect_producer_stats(m, size);
	obj = mut_resources_create_resource(&m->resources, obj_addr);
	if (obj == (mut_resource *)0)
		return 0;
	if (!mut_resource_is_new(obj)) {
		mut_log_error(m->log, "CMM");
		mut_log_string(m->log, calloc->name);
		mut_log_string(m->log, mut_resource_producer_name(obj));
		mut_log_exec_addr(m->log, obj_addr);
		(void)mut_log_end(m->log);
		return 0;
	} 
	mut_resource_make_producer(obj, calloc, size, bt);
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, calloc->name);
		mut_log_uint(m->log, n_elems);
		mut_log_uint(m->log, elem_size);
		mut_log_exec_addr(m->log, obj_addr);
		if (m->flags & mut_control_backtrace)
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	if (m->flags & mut_control_usage)
		mut_report_usage(m);
	return 1;
}



/*
 *
 *
 */
static int mut_do_calloc(mut *m, mut_exec_addr pc, mut_action *calloc)
{
	mut_arg         elem_size;
	mut_arg         n_elems;
	mut_exec_addr   return_addr;
	mut_arg         obj_addr_arg;
	mut_exec_addr   obj_addr;
	mut_backtrace * bt;
	size_t          adjusted_size;

	if (!mut_process_read_arg(&m->process, 0, &n_elems))
		return 0;
	if (!mut_process_read_arg(&m->process, 1, &elem_size))
		return 0;
	if (!mut_produce_backtrace(m, &bt))
		return 0;
	if (!mut_process_function_return_addr(&m->process, &return_addr))
		goto free_backtrace;
	if (m->flags & mut_control_check) {
		mut_arg total_size = n_elems*elem_size;
		mut_arg n_zones = total_size/mut_zone_size + 1 + 2;
		adjusted_size = n_zones*mut_zone_size;
		if (!mut_process_write_arg(&m->process, 0, n_zones))
			goto free_backtrace;
		if (!mut_process_write_arg(&m->process, 1, mut_zone_size))
			goto free_backtrace;
	}
	if (!mut_do_action(m, return_addr, calloc))
		goto free_backtrace;
	if (!mut_process_read_result(&m->process, &obj_addr_arg))
		goto free_backtrace;
	if (m->flags & mut_control_stats)
		calloc->hits = mut_counter_inc(calloc->hits, 1);
	if (obj_addr_arg == 0) {
		mut_calloc_failure(m, n_elems, elem_size, bt);
		return 1;
	} 
	obj_addr = mut_exec_addr_from_int(obj_addr_arg);
	if ((m->flags & mut_control_check)
	    &&  !mut_install_check_zones(m, &obj_addr, adjusted_size))
		goto free_backtrace;
	return mut_calloc_success(m, n_elems, elem_size, obj_addr, bt);

free_backtrace:
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	return 0;
}




/*
 * `mut_log_fum' is called when free (3), cfree(3) or 
 * realloc(3) has been called on an unallocated address.
 * `mut_log_fum' logs an FUM error with the error
 * logger (including a backtrace at the current point if a backtrace 
 * is requested).
 */
static void mut_log_fum(mut *m, mut_exec_addr obj_addr,
			mut_action *consumer,
			mut_backtrace *bt)
{
	mut_log_error(m->log, "FUM");
	mut_log_string(m->log, consumer->name);
	mut_log_exec_addr(m->log, obj_addr);
	if (m->flags & mut_control_backtrace)
		mut_log_backtrace(m->log, bt);
	(void)mut_log_end(m->log);
}



/*
 * `mut_log_ffm' is called when free, cfree or realloc is called on an
 * object that has already been freed.  It logs an FFM error with the
 * error logger (and if backtraces are requested, a backtrace of the
 * previous free, cfree or realloc).
 */
static void mut_log_ffm(mut *m, mut_exec_addr obj_addr,
			mut_action *consumer, mut_resource *obj,
			mut_backtrace * bt)
{
	mut_assert_pre(!(m->flags & mut_control_free));
	mut_assert_pre(mut_resources_resource_deleted(&m->resources, obj));

	mut_log_error(m->log, "FFM");
	mut_log_string(m->log, consumer->name);
	mut_log_exec_addr(m->log, obj_addr);
	if (m->flags & mut_control_backtrace)
		mut_log_backtrace(m->log, bt);
	mut_log_string(m->log, mut_resource_consumer_name(obj));
	if (m->flags & mut_control_backtrace)
		mut_log_backtrace(m->log, obj->consumer.backtrace);
	(void)mut_log_end(m->log);
}



/*
 * `mut_log_wum' is called when free, cfree or realloc is called on an
 * object and a write to the pre or post zone has been detected.  It logs an
 * WUM error with the error logger (and if backtraces are requested,
 * a backtrace of the malloc, calloc or realloc that created the object).
 */
static void mut_log_wum(mut *m, mut_exec_addr obj_addr, mut_exec_addr zone_addr,
			int zone_value, mut_action *consumer, mut_resource *obj,
			mut_backtrace *bt)
{
	mut_assert_pre(m->flags & mut_control_check);

	mut_log_error(m->log, "WUM");
	mut_log_string(m->log, consumer->name);
	mut_log_exec_addr(m->log, obj_addr);
	mut_log_ulong(m->log, (unsigned long)obj->size);
	mut_log_exec_addr(m->log, zone_addr);
	mut_log_uint_hex(m->log, zone_value);
	if (m->flags & mut_control_backtrace)
		mut_log_backtrace(m->log, bt);
	mut_log_string(m->log, mut_resource_producer_name(obj));
	if (m->flags & mut_control_backtrace)
		mut_log_backtrace(m->log, obj->producer.backtrace);
	(void)mut_log_end(m->log);
}



/*
 * `mut_zone_check' checks that the zones on either side of `obj' at
 * `obj_addr'.  If they have been corrupted a WUM error is logged otherwise
 * nothing is logged.
 *
 * `mut_zone_check' returns a non-zero value if the check cannot be done.
 */
static int mut_zone_check(mut *m, mut_exec_addr obj_addr, mut_resource *obj,
			  mut_action *consumer, mut_backtrace *bt)
{
	mut_reg zone_value;
	size_t true_size = mut_round_up(obj->size);
	size_t pre_zone_addr = obj_addr-mut_zone_size;
	size_t post_zone_addr = pre_zone_addr+true_size-mut_zone_size;

	if (!mut_process_read_word(&m->process, pre_zone_addr, &zone_value))
		return 0;
	if (zone_value != mut_zone_value) {
		mut_log_wum(m, obj_addr, pre_zone_addr, zone_value, consumer, obj, bt);
		return 0;
	}
	if (!mut_process_read_word(&m->process, post_zone_addr, &zone_value))
		return 0;
	if (zone_value != mut_zone_value) {
		mut_log_wum(m, obj_addr, post_zone_addr, zone_value, consumer, obj, bt);
		return 0;
	}
	return 1;
}


/*
 * `mut_do_consumer' is called when either free or cfree has been called
 * in the target program.  Exactly what happens depend the argument to
 * the target [c]free and the control flags passed to `mut_open'.
 *
 *.free.0: if the argument to [c]free is 0 then ...
 *.free.0.trace: if tracing of calls is requested, the free call is logged.
 *.free.0.do: the target [c]free is performed.
 *
 *.free.addr: If the argument to [c]free is not 0 then ...
 *.free.addr.alloc: If the address has been allocated then ...
 *.free.addr.alloc.trace: If tracing is requested, the free call is logged.
 *.free.addr.alloc.free: If the address has already been freed ..
 *.free.addr.alloc.free.log: An FFM error is logged.
 *.free.addr.alloc.free.free: If target memory is being freed, then the target
 * [c]free call is run as normal.  If target memory is not being freed then
 * the [c]free call is skipped.
 *.free.addr.alloc.avail: If the address has been allocated and is avilable ...
 *.free.addr.alloc.avail.stats: The statistics are updated if necessary.
 *.free.addr.alloc.avail.check: If pre and post zones are being used, check
 * that they are undisturbed.
 *.free.addr.alloc.avail.free: If target memory is being freed, then the
 * resource is removed from the resource map and the target [c]free call
 * is run as normal.  If target memory is not being freed the resource
 * is not removed and the the [c]free call is skipped.
 *
 *.free.addr.unalloc: If the address has not been allocated then ...
 *.free.addr.unalloc.trace: If tracing is requested, the free call is logged.
 *.free.addr.unalloc.log: A FUM error is logged.
 *.free.addr.unalloc.free: If target memory is being freed, then the target
 * [c]free call is run as normal.  If target memory is not being freed then
 * the [c]free call is skipped.
 *
 * `mut_do_consumer' returns a non-zero value on success and zero if there
 * is a fatal error.
 */
static int mut_do_consumer(mut *m, mut_exec_addr pc, mut_action *consumer)
{
	mut_arg         obj_addr_arg;
	mut_exec_addr   obj_addr;
	mut_exec_addr   return_addr;
	mut_resource  * obj;
	mut_backtrace * bt;

	if (m->flags & mut_control_stats)
		consumer->hits = mut_counter_inc(consumer->hits, 1);

	if (!mut_process_read_arg(&m->process, 0, &obj_addr_arg))
		return 0;
	obj_addr = mut_exec_addr_from_int(obj_addr_arg);
	if (!mut_process_function_return_addr(&m->process, &return_addr))
		return 0;
	if (!mut_produce_backtrace(m, &bt))
		return 0;

	if (obj_addr == 0) {
		/* .free.0 */
		if (m->flags & mut_control_trace) {
			/* .free.0.trace */
			mut_log_info(m->log, "trace");
			mut_log_string(m->log, consumer->name);
			mut_log_exec_addr(m->log, obj_addr);
			(void)mut_log_end(m->log);
		}
		if (m->flags & mut_control_backtrace)
			mut_backtrace_close(bt);
		/* .free.0.do */
		return mut_do_action(m, return_addr, consumer);
	}

	/* .free.addr */
	obj = mut_resources_lookup(&m->resources, obj_addr);
	if (obj == (mut_resource *)0) {
		/* .free.addr.unalloc */
		if (m->flags & mut_control_trace) {
			/* .free.addr.unalloc.trace */
			mut_log_info(m->log, "trace");
			mut_log_string(m->log, consumer->name);
			mut_log_exec_addr(m->log, obj_addr);
			mut_log_int(m->log, 0);	/* indicate no obj */
			(void)mut_log_end(m->log);
		}
		mut_log_fum(m, obj_addr, consumer, bt);
		if (m->flags & mut_control_backtrace)
			mut_backtrace_close(bt);
		return (m->flags & mut_control_free)
			? mut_do_action(m, return_addr, consumer)
			: mut_process_skip_to(&m->process, return_addr);
	}

	if (m->flags & mut_control_trace) {
		/* .free.addr.alloc.trace */
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, consumer->name);
		mut_log_exec_addr(m->log, obj_addr);
		mut_log_ulong(m->log, (unsigned long)obj->size);
		(void)mut_log_end(m->log);
	}
	if (mut_resources_resource_deleted(&m->resources, obj)) {
		/* .free.addr.alloc.free */
		/* .free.addr.alloc.free.log */
		mut_log_ffm(m, obj_addr, consumer, obj, bt);
		if (m->flags & mut_control_backtrace)
			mut_backtrace_close(bt);
		/* .free.addr.alloc.free.free */
		return (m->flags & mut_control_free)
			? mut_do_action(m, return_addr, consumer)
			: mut_process_skip_to(&m->process, return_addr);
	} 

	/* .free.addr.alloc.avail */
	/* .free.addr.alloc.avail.stats */
	if (m->flags & mut_control_stats) {
		m->stats.now.space = mut_counter_dec(m->stats.now.space, obj->size);
		m->stats.now.count = mut_counter_dec(m->stats.now.count, 1);
	}

	/* .free.addr.alloc.avail.check */
	if ((m->flags & mut_control_check)
	    &&  !mut_zone_check(m, obj_addr, obj, consumer, bt))
		goto free_backtrace;

	/* .free.addr.alloc.avail.free */
	if ((m->flags & mut_control_zero)
	    &&  !mut_process_memset(&m->process, obj_addr, 0, obj->size))
		goto free_backtrace;

	mut_resources_delete_resource(&m->resources, obj, consumer, bt);
	obj = (mut_resource *)0;
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	if (m->flags & mut_control_usage)
		mut_report_usage(m);
	return (m->flags & mut_control_free)
		? mut_do_action(m, return_addr, consumer)
		: mut_process_skip_to(&m->process, return_addr);

free_backtrace:
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	return 0;
}



/*
 * `mut_realloc_failure' is called when realloc returns 0.  It 
 * simply logs a warning.  If tracing is enabled, a trace is
 * is also logged, optionally with a backtrace.
 *
 * `mut_realloc_failure' is passed the ownership of `bt' which means it
 * is up to `mut_realloc_failure' to close `bt' if it is no longer needed.
 */
static void
mut_realloc_failure(mut *m, size_t new_size, mut_exec_addr old_addr,
		    size_t old_size, mut_backtrace *bt)
{
	mut_action *realloc = &m->actions.realloc;

	mut_assert_pre((m->flags & mut_control_backtrace) || (bt == 0));
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, realloc->name);
		mut_log_exec_addr(m->log, old_addr);
		mut_log_uint(m->log, (unsigned long)old_size);
		mut_log_uint(m->log, (unsigned long)new_size);
		mut_log_exec_addr(m->log, 0);
		if (m->flags & mut_control_backtrace) 
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	mut_log_warning(m->log, "RR0");
	mut_log_string(m->log, realloc->name);
	mut_log_exec_addr(m->log, old_addr);
	mut_log_uint(m->log, (unsigned long)old_size);
	mut_log_uint(m->log, (unsigned long)new_size);
	if (m->flags & mut_control_backtrace) 
		mut_log_backtrace(m->log, bt);
	(void)mut_log_end(m->log);
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
}




/*
 * `mut_realloc_malloc' is called when a realloc call of the form 
 * realloc(0, n) has been encountered, i.e. the realloc is effectively
 * a malloc.
 *
 * `mut_realloc_malloc' creates a new resource from the address returned
 * by realloc and adds it to the resources.
 *
 * `mut_realloc_malloc' also takes care of collecting any necessary
 * allocation statistics.
 *
 * if tracing is enabled, then `mut_realloc_malloc' logs a trace
 * for realloc with the logger.
 *
 * `mut_realloc_malloc' returns a non-zero value on success and zero
 * on failure.
 */
static int mut_realloc_malloc(mut *m, size_t obj_size, mut_backtrace *bt)
{
	mut_action   * realloc = &m->actions.realloc;
	mut_arg        obj_addr_arg;
	mut_exec_addr  obj_addr;
	mut_resource * obj;
	mut_assert_pre((m->flags & mut_control_backtrace) || (bt == 0));

	if (!mut_process_read_result(&m->process, &obj_addr_arg))
		goto free_backtrace;
	if (m->flags & mut_control_stats)
		realloc->hits = mut_counter_inc(realloc->hits, 1);
	if (obj_addr_arg == 0) {
		mut_realloc_failure(m, obj_size, 0, 0, bt);
		return 0;
	}
	obj_addr = mut_exec_addr_from_int(obj_addr_arg);
	if ((m->flags & mut_control_check)
	    &&  !mut_install_check_zones(m, &obj_addr, mut_round_up(obj_size)))
		goto free_backtrace;
	obj = mut_resources_create_resource(&m->resources, obj_addr);
	if (obj == (mut_resource *)0)
		goto free_backtrace;
	mut_assert_check(mut_resource_is_new(obj));
	if (m->flags & mut_control_stats)
		mut_collect_producer_stats(m, obj_size);
	obj->producer.action = realloc;
	obj->size = obj_size;
	if (m->flags & mut_control_backtrace)
		obj->producer.backtrace = bt;
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, realloc->name);
		mut_log_exec_addr(m->log, 0);
		mut_log_ulong(m->log, 0);
		mut_log_ulong(m->log, (unsigned long)obj_size);
		mut_log_exec_addr(m->log, obj_addr);
		if (m->flags & mut_control_backtrace)
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	if (m->flags & mut_control_usage)
		mut_report_usage(m);
	return 1;

free_backtrace:
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	return 0;
}



/*
 * `mut_realloc_realloc' is called when a realloc with a non-zero address
 * has been encountered and the user has requested that memory should really
 * freed when free(3), cfree(3) or realloc(3) is called.
 *
 * `mut_realloc_realloc' updates `obj' with the new object address
 * (which may be the same as obj->addr in the case were realloc does
 * not allocate memory but instead extends the existing object), sets its
 * associated action to be `realloc' and updates the object size.
 *
 * `mut_realloc_realloc' also takes care of collecting any necessary
 * allocation statistics.
 *
 * if tracing is enabled, then `mut_realloc_realloc' logs a trace
 * for realloc with the logger.
 *
 * `mut_realloc_realloc' returns a non-zero value on success and zero
 * on failure.
 */
static int mut_realloc_realloc(mut *m, mut_resource *obj,
			       size_t new_obj_size, mut_backtrace *bt)
{
	mut_action    * realloc = &m->actions.realloc;
	mut_arg         new_obj_addr;
	size_t          old_obj_size;
	mut_exec_addr   old_obj_addr;

	mut_assert_pre(m->flags & mut_control_free);
	mut_assert_pre((m->flags & mut_control_backtrace) || (bt == 0));

	if (!mut_process_read_result(&m->process, &new_obj_addr))
		return 0;

	if (m->flags & mut_control_stats)
		realloc->hits = mut_counter_inc(realloc->hits, 1);
	if (new_obj_addr == 0) {
		mut_realloc_failure(m, new_obj_size, obj->addr, obj->size, bt);
		return 0;
	}
	old_obj_size = obj->size;
	old_obj_addr = obj->addr;

	if (m->flags & mut_control_stats) {
		m->stats.now.space = mut_counter_dec(m->stats.now.space, old_obj_size);
		m->stats.now.count = mut_counter_dec(m->stats.now.count, 1);
	}

	if (old_obj_addr == new_obj_addr) {
		mut_assert_check(!mut_resources_resource_deleted(&m->resources, obj));
	} else {
		if (m->flags & mut_control_zero) {
			if (!mut_process_memset(&m->process, old_obj_addr, 0, old_obj_size))
				return 0;
		}
		obj->addr = new_obj_addr;
	} 
	if (m->flags & mut_control_stats)
		mut_collect_producer_stats(m, new_obj_size);
	/*
	 * Need to clean up the following so it doesn't break abstractions ...
	 */
	obj->producer.action = realloc;
	obj->size = new_obj_size;
	obj->consumer.action = (mut_action *)0;
	if (m->flags & mut_control_backtrace) {
		if (obj->consumer.backtrace)
			mut_backtrace_close(obj->consumer.backtrace);
		obj->consumer.backtrace = (mut_backtrace *)0;
		mut_backtrace_close(obj->producer.backtrace);
		obj->producer.backtrace = bt;
	}
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, realloc->name);
		mut_log_exec_addr(m->log, old_obj_addr);
		mut_log_ulong(m->log, (unsigned long)old_obj_size);
		mut_log_ulong(m->log, (unsigned long)new_obj_size);
		mut_log_exec_addr(m->log, new_obj_addr);
		if (m->flags & mut_control_backtrace)
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	if (m->flags & mut_control_usage)
		mut_report_usage(m);
	return 1;
}



/*
 * `mut_realloc_realloc_no_free' is called when a realloc with a legal
 * non-zero address has been encountered and the user has requested
 * that memory should not be freed.
 *
 * if tracing is enabled, then `mut_realloc_realloc_no_free' logs a trace
 * for realloc with the logger.
 *
 * `mut_realloc_realloc_no_free' is passed the ownership of `bt' and so it is
 * the responsibility of `mut_realloc_realloc_no_free' to close it.
 *
 * `mut_realloc_realloc_no_free' returns a non-zero value on success and zero
 * on failure.
 */
static int mut_realloc_realloc_no_free(mut *m, mut_resource *old_obj,
				       size_t new_size, mut_backtrace *bt)
{
	mut_action   * realloc = &m->actions.realloc;
	mut_arg        obj_addr;
	mut_resource * obj;
	size_t         sz;
	mut_exec_addr  old_obj_addr = old_obj->addr;
	size_t         old_obj_size = old_obj->size;

	mut_assert_pre((m->flags & mut_control_backtrace) || (bt == 0));

	if (!mut_process_read_result(&m->process, &obj_addr))
		goto free_backtrace;
	mut_assert_check(old_obj_addr != obj_addr);
	if (obj_addr == 0) {
		mut_realloc_failure(m, new_size, old_obj_addr, old_obj_size, bt);
		return 0;
	}
	if (m->flags & mut_control_stats) {
		m->stats.now.space = mut_counter_dec(m->stats.now.space, old_obj_size);
		m->stats.now.count = mut_counter_dec(m->stats.now.count, 1);
	}
	mut_resources_delete_resource(&m->resources, old_obj, realloc, bt);
	old_obj = (mut_resource *)0;

#if XXX
	if ((m->flags & mut_control_check)
	    &&  !mut_install_check_zones(m, &obj_addr, mut_round_up(obj_size)))
		goto free_backtrace;
#endif

	if ((obj = mut_resources_create_resource(&m->resources, obj_addr)) == 0)
		goto free_backtrace;
	mut_assert_check(mut_resource_is_new(obj));
	mut_resource_make_producer(obj, realloc, new_size, bt);

	sz = (old_obj_size < new_size) ? old_obj_size : new_size;
	if (!mut_process_memcpy(&m->process, obj->addr, old_obj_addr, sz))
		return 0;
	if ((m->flags & mut_control_zero)
	    && !mut_process_memset(&m->process, old_obj_addr, 0, old_obj_size))
		return 0;

	if (m->flags & mut_control_stats) {
		realloc->hits = mut_counter_inc(realloc->hits, 1);
		mut_collect_producer_stats(m, new_size);
	}
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, realloc->name);
		mut_log_exec_addr(m->log, old_obj_addr);
		mut_log_ulong(m->log, (unsigned long)old_obj_size);
		mut_log_ulong(m->log, (unsigned long)new_size);
		mut_log_exec_addr(m->log, obj->addr);
		if (m->flags & mut_control_backtrace)
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}
	if (m->flags & mut_control_usage)
		mut_report_usage(m);
	return 1;

free_backtrace:
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	return 0;
}



/*
 * `mut_realloc_malloc_no_free' is called on exit of a realloc(p, n) 
 * where p is not the address of memory previously allocated (i.e. a FUM 
 * error) and the user has indicated that memory should not be freed.
 *
 * The caller of `mut_realloc_malloc_no_free' ensures that the bogus address
 * is replaced by a 0, thereby ensuring that the target process interprets
 * the call as a realloc(0, n) i.e. an implied malloc.
 *
 * `mut_realloc_malloc_no_free' picks up the result and ...
 *
 * if tracing is enabled, then `mut_realloc_malloc_no_free' logs a trace
 * for realloc with the logger.
 *
 * `mut_realloc_malloc_no_free' is passed the ownership of `bt' and so it is
 * the responsibility of `mut_realloc_malloc_no_free' to close it or pass the
 * ownership elsewhere.
 *
 * `mut_realloc_malloc_no_free' returns a non-zero value on success and zero
 * on failure.
 */
static int mut_realloc_malloc_no_free(mut *m, size_t new_size, mut_backtrace *bt)
{
	mut_action    * realloc = &m->actions.realloc;
	mut_arg         obj_addr_arg;
	mut_exec_addr   obj_addr;
	mut_resource  * obj;

	mut_assert_pre((m->flags & mut_control_backtrace) || (bt == 0));

	if (!mut_process_read_result(&m->process, &obj_addr_arg))
		goto free_backtrace;
	if (obj_addr_arg == 0) {
		mut_realloc_failure(m, new_size, 0, 0, bt);
		return 0;
	}
	obj_addr = mut_exec_addr_from_int(obj_addr_arg);
	if ((m->flags & mut_control_check)
	    &&  !mut_install_check_zones(m, &obj_addr, mut_round_up(new_size)))
		goto free_backtrace;
	if ((obj = mut_resources_create_resource(&m->resources, obj_addr)) == 0)
		goto free_backtrace;
	mut_assert_check(mut_resource_is_new(obj));
	mut_resource_make_producer(obj, realloc, new_size, bt);

	if (m->flags & mut_control_stats) {
		realloc->hits = mut_counter_inc(realloc->hits, 1);
		mut_collect_producer_stats(m, obj->size);
	}
	if (m->flags & mut_control_trace) {
		mut_log_info(m->log, "trace");
		mut_log_string(m->log, realloc->name);
		mut_log_exec_addr(m->log, 0);
		mut_log_ulong(m->log, 0);
		mut_log_ulong(m->log, (unsigned long)obj->size);
		mut_log_exec_addr(m->log, obj->addr);
		if (m->flags & mut_control_backtrace)
			mut_log_backtrace(m->log, bt);
		(void)mut_log_end(m->log);
	}

	return 1;

free_backtrace:
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	return 0;
}



static void mut_realloc_fum_trace(mut *m, char const *realloc_name,
                                  mut_exec_addr obj_addr, 
                                  unsigned int new_size)
{
	mut_log_info(m->log, "trace");
	mut_log_string(m->log, realloc_name);
	mut_log_exec_addr(m->log, obj_addr);
	mut_log_uint(m->log, new_size);
	mut_log_int(m->log, 0);	/* indicate no obj */
	(void)mut_log_end(m->log);
}



/*
 *
 *
 *
 *
 */
static int mut_do_realloc(mut *m, mut_exec_addr pc, mut_action *realloc)
{
	mut_arg         obj_addr_arg;
	mut_arg         new_size;
	mut_exec_addr   obj_addr;
	mut_exec_addr   return_addr;
	mut_resource  * obj;
	mut_backtrace * bt;

	if (!mut_process_read_arg(&m->process, 0, &obj_addr_arg))
		return 0;
	obj_addr = mut_exec_addr_from_int(obj_addr_arg);
	if (!mut_process_read_arg(&m->process, 1, &new_size))
		return 0;
	if (!mut_process_function_return_addr(&m->process, &return_addr))
		return 0;
	if (!mut_produce_backtrace(m, &bt))
		return 0;

	if (obj_addr == 0) {
		if ((m->flags & mut_control_check)
		    &&  !mut_process_write_arg(&m->process, 1, mut_round_up(new_size)))
			goto free_backtrace;
		if (!mut_do_action(m, return_addr, realloc))
			goto free_backtrace;
		return mut_realloc_malloc(m, (size_t)new_size, bt);
	}

	if ((obj = mut_resources_lookup(&m->resources, obj_addr)) == 0) {
		if (m->flags & mut_control_stats)
			realloc->hits = mut_counter_inc(realloc->hits, 1);
		mut_log_fum(m, obj_addr, realloc, bt);
		if (m->flags & mut_control_free) {
			if (m->flags & mut_control_trace)
				mut_realloc_fum_trace(m, realloc->name, obj_addr, new_size);
			if (m->flags & mut_control_backtrace)
				mut_backtrace_close(bt);
			return mut_do_action(m, return_addr, realloc);
		} else {
			if (!mut_process_write_arg(&m->process, 0, 0))
				goto free_backtrace;
			if ((m->flags & mut_control_check)
			    && !mut_process_write_arg(&m->process, 1, mut_round_up(new_size)))
				goto free_backtrace;
			if (!mut_do_action(m, return_addr, realloc))
				goto free_backtrace;
			return mut_realloc_malloc_no_free(m, (size_t)new_size, bt);
		}
	}

	if (mut_resources_resource_deleted(&m->resources, obj))
		mut_log_ffm(m, obj_addr, realloc, obj, bt);

	if (!(m->flags & mut_control_free)
	    &&  !mut_process_write_arg(&m->process, 0, 0))
		goto free_backtrace;
	if ((m->flags & mut_control_check)
	    && !mut_process_write_arg(&m->process, 1, mut_round_up(new_size)))
		goto free_backtrace;
	if (!mut_do_action(m, return_addr, realloc))
		goto free_backtrace;
	return (m->flags & mut_control_free)
		? mut_realloc_realloc(m, obj, (size_t)new_size, bt)
		: mut_realloc_realloc_no_free(m, obj, (size_t)new_size, bt);

free_backtrace:
	if (m->flags & mut_control_backtrace)
		mut_backtrace_close(bt);
	return 0;
}


static int mut_do_usage(mut *m, mut_exec_addr pc, mut_action *usage)
{
	mut_exec_addr return_addr;

	mut_report_usage(m);
	if (!mut_process_function_return_addr(&m->process, &return_addr))
		return 0;
	return mut_do_action(m, return_addr, usage);
}


static int mut_remove_action_breakpoints(mut *m)
{
	mut_action *a = m->actions.root;

	for (; a != (mut_action *)0; a = a->next) {
		if (!mut_process_restore(&m->process, a->addr, a->instr))
			return 0;
	}
	return 1;
}



static int mut_install_action_breakpoints(mut *m)
{
	mut_action *a = m->actions.root;
	for (; a != (mut_action *)0; a = a->next) {
		if (!mut_process_breakpoint(&m->process, a->addr, &a->instr))
			return 0;
	}
	return 1;
}



int mut_run(mut *m)
{
	int            status;
	mut_action    *action;
	mut_exec_addr  pc;

	for (;;) {
		if (!mut_process_resume(&m->process))
			return 0;
		status = mut_process_wait(&m->process, &pc);
		if (status < 0)
			return 1;
		if (status == 0)
			return 0;
		action = mut_actions_lookup(m, pc);
		if (action == 0)
			continue;
		if (!mut_remove_action_breakpoints(m))
			return 0;
		switch (action->type) {
		case mut_action_type_malloc:
			if (!mut_do_malloc(m, pc, action))
				return 0;
			break;
		case mut_action_type_consumer:
			if (!mut_do_consumer(m, pc, action))
				return 0;
			break;
		case mut_action_type_realloc:
			if (!mut_do_realloc(m, pc, action))
				return 0;
			break;
		case mut_action_type_calloc:
			if (!mut_do_calloc(m, pc, action))
				return 0;
			break;
		case mut_action_type_usage:
			if (!mut_do_usage(m, pc, action))
				return 0;
			break;
		}
		if (!mut_install_action_breakpoints(m))
			return 0;
	}
}



static void mut_log_stats(mut *m)
{
	mut_log_info(m->log, "stats");
	mut_log_string(m->log, "alloc.now");
	mut_log_counter(m->log, m->stats.now.space);
	mut_log_counter(m->log, m->stats.now.count);
	mut_log_string(m->log, "alloc.total");
	mut_log_counter(m->log, m->stats.total.space);
	mut_log_counter(m->log, m->stats.total.count);
	mut_log_string(m->log, "alloc.max.space");
	mut_log_counter(m->log, m->stats.max.space.space);
	mut_log_counter(m->log, m->stats.max.space.count);
	mut_log_string(m->log, "alloc.max.count");
	mut_log_counter(m->log, m->stats.max.count.space);
	mut_log_counter(m->log, m->stats.max.count.count);
	mut_log_string(m->log, "malloc");
	mut_log_counter(m->log, m->actions.malloc.hits);
	mut_log_string(m->log, "free");
	mut_log_counter(m->log, m->actions.free.hits);
	mut_log_string(m->log, "realloc");
	mut_log_counter(m->log, m->actions.realloc.hits);
	mut_log_string(m->log, "calloc");
	mut_log_counter(m->log, m->actions.calloc.hits);
	mut_log_string(m->log, "cfree");
	mut_log_counter(m->log, m->actions.cfree.hits);
	(void)mut_log_end(m->log);
}



/*
 * `mut_log_leaks' logs any leaks detected by MUT.
 */
static void mut_log_leaks(mut * m)
{
	mut_resource * r;
	mut_resources_iter iter;

	(void)mut_resources_iter_open(&m->resources, &iter, 0);
	while ((r = mut_resources_iter_read(&iter)) != (mut_resource *)0) {
		mut_log_error(m->log, "MLK");
		mut_log_string(m->log, mut_resource_producer_name(r));
		mut_log_ulong(m->log, (unsigned long)r->size);
		mut_log_exec_addr(m->log, r->addr);
		if (m->flags & mut_control_backtrace)
			mut_log_backtrace(m->log, r->producer.backtrace);
		(void)mut_log_end(m->log);
	}
	mut_resources_iter_close(&iter);
}



int mut_close(mut *m)
{
	mut_log_leaks(m);
	if (m->flags & mut_control_stats)
		mut_log_stats(m);
	mut_resources_close(&m->resources);
	mut_process_close(&m->process);
	return mut_exec_close(&m->exec);
}



void mut_close_on_error(mut *m)
{
	mut_resources_close(&m->resources);
	mut_process_close(&m->process);
	(void)mut_exec_close(&m->exec);
}

/*
**.intro: XXX
**
*/

#include "mut_stddef.h"		/* size_t */
#include "mut_assert.h"		/* mut_assert_check */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log.h"		/* mut_log */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_mem.h"		/* mut_mem_malloc, mut_mem_free */
#include "mut_errno.h"		/* errno */
#include "mut_instr.h"		/* mut_instr */
#include "mut_backtrace_close.h" /* mut_backtrace_close */
#include "mut_backtrace_sharer.h"
#include "mut_action.h"
#include "mut_resources.h"

void mut_resource_make_producer(mut_resource  * obj,
				mut_action    * producer,
				size_t          obj_size,
				mut_backtrace * bt)
{
	obj->producer.action = producer;
	obj->producer.backtrace = bt;
	obj->size = obj_size;
}


/*
 * Deliberately doesn't do any sort of lazy freeing to avoid any 
 * problems with access errors being masked.
 *
 * Profile and rewrite if necessary once the code is stable.
 *
 * Note that unlike `mut_resources_mark_as_free' this does not
 * adjust the n_consumed_elems field since this is not relevant
 * if deletion is really being done.
 */
static void mut_resources_really_delete_resource(mut_resources *rs,
						 mut_resource *obj,
						 mut_action *consumer,
						 mut_backtrace * bt)
{
	mut_exec_addr addr = obj->addr;
	mut_resource ** rh= &rs->elems;
	mut_resource * r= *rh;

	mut_assert_pre(obj->consumer.action == (mut_action *)0);
	mut_assert_pre(rs->n_active_elems > 0);

loop:
	mut_assert_check(r != (mut_resource *)0);
	if (r->addr == addr) {
		*rh= r->next;
		if (r->producer.backtrace)
			mut_backtrace_close(r->producer.backtrace);
		mut_mem_free(r);
		rs->n_active_elems -= 1;
	} else {
		rh= &r->next;
		r= *rh;
		goto loop;
	}
}


static void mut_resources_mark_as_free(mut_resources *rs,
				       mut_resource  *obj,
				       mut_action    *consumer,
				       mut_backtrace *bt)
{
	mut_assert_pre(obj->consumer.action == (mut_action *)0);
	mut_assert_pre(rs->n_active_elems > 0);

	obj->consumer.action= consumer;
	if (bt != (mut_backtrace *)0)
		obj->consumer.backtrace= mut_backtrace_share(bt);
	rs->n_active_elems -= 1;
	rs->n_consumed_elems += 1;
}


void mut_resources_open(mut_resources *rs, mut_log *log, int really_free)
{
	rs->log = log;
	rs->elems = (mut_resource *)0;
	rs->n_active_elems = 0;
	rs->n_consumed_elems = 0;
	rs->delete = really_free 
		? mut_resources_really_delete_resource
		: mut_resources_mark_as_free;
}


mut_resource *mut_resources_create_resource(mut_resources * rs, mut_exec_addr addr)
{
	mut_resource *r = rs->elems;
loop:
	if (r == (mut_resource *)0) {
		if ((r= mut_mem_malloc(sizeof(mut_resource))) == (mut_resource *)0) {
			mut_log_mem_full(rs->log, errno);
			return (mut_resource *)0;
		}
		r->addr = addr;
		r->next = rs->elems;
		r->producer.action = (mut_action *)0;
		r->producer.backtrace = (mut_backtrace *)0;
		r->consumer.action = (mut_action *)0;
		r->consumer.backtrace = (mut_backtrace *)0;
		rs->elems = r;
		rs->n_active_elems += 1;
		return r;
	} else if (r->addr == addr) {
		return r;
	} else {
		r= r->next;
		goto loop;
	}
}


mut_resource *mut_resources_lookup(mut_resources *rs, mut_exec_addr addr)
{
	mut_resource * r= rs->elems;
loop:
	if (r == (mut_resource *)0) {
		return r;
	} else if (r->addr == addr) {
		return r;
	} else {
		r= r->next;
		goto loop;
	}
}


size_t mut_resources_iter_open(mut_resources *rs, mut_resources_iter *iter, int all)
{
	iter->elem = rs->elems;
	iter->all = all;
	return rs->n_active_elems + (all ? rs->n_consumed_elems : 0);
}



mut_resource *mut_resources_iter_read(mut_resources_iter *iter)
{
	int all = iter->all;
	mut_resource *item;
next:
	item = iter->elem;
	if (item == (mut_resource *)0)
		return item;
	iter->elem = item->next;
	if (!all && (item->consumer.action != (mut_action *)0))
		goto next;
	return item;
}


void mut_resources_iter_close(mut_resources_iter *iter)
{
	iter->elem= (mut_resource *)0;
}



void mut_resources_close(mut_resources *rs)
{
	mut_resource *r = rs->elems;

	while (r != (mut_resource *)0) {
		mut_resource *next = r->next;
		if (r->producer.backtrace)
			mut_backtrace_close(r->producer.backtrace);
		if (r->consumer.backtrace)
			mut_backtrace_close(r->consumer.backtrace);
		mut_mem_free(r);
		r= next;
	}
}

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

void mut_resource_make_producer(mut_resource  *obj,
				mut_action    *producer,
				size_t         obj_size,
				mut_backtrace *bt)
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
						 mut_resource  *obj,
						 mut_action    *consumer,
						 mut_backtrace *bt)
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


mut_resources_private void
mut_resources_mark_as_free(mut_resources *rs, mut_resource  *obj, mut_action *consumer, mut_backtrace *bt)
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
	rs->log= log;
	rs->elems= (mut_resource *)0;
	rs->n_active_elems= 0;
	rs->n_consumed_elems= 0;
	rs->delete= really_free 
		? mut_resources_really_delete_resource
		: mut_resources_mark_as_free;
}


static mut_resource *mut_resources_internal_add(mut_resource **nn, mut_exec_addr addr,
						int *balanced, mut_resource *parent)
{
	mut_resource * r;
	mut_resource * n= *nn;
	mut_resource *p1;
	mut_resource *p2;

	if (n == 0) {
		n= malloc(sizeof(*n));
		if (n == 0)
			return 0;
		n->left= 0;
		n->right= 0;
		n->balanced= 0;
		n->addr= addr;
		n->parent= parent;
		n->producer.action= 0;
		n->producer.backtrace= 0;
		n->consumer.action= 0;
		n->consumer.backtrace= 0;
		*balanced= 1;
		*nn= n;
		return n;
	}

	if (mut_exec_addr_lt(addr, n->addr)) {
		r= mut_resource_internal_add(&n->left, addr, balanced, n);
		if (r == 0)
			return r;
		if (*balanced) {
			int n_balanced= n->balanced;
			if (n_balanced > 0) {
				n->balanced= 0;
				*balanced= 0;
			} else if (n_balanced == 0) {
				n->balanced= -1;
			} else /* n_balanced < 0 */ {
				p1= n->left;
				if (p1->balanced < 0) {	/* LR */
					mut_resource * tp= n->parent;
					p1->right->parent= n;
					n->parent= p1->parent;
					p1->parent= tp;
					n->left= p1->right;
					p1->right= n;
					n->balanced= 0;
					*nn= p1;
				} else {		/* double LR */
					p2= p1->right;
					p2->parent= n->parent;
					p1->parent= p2;
					p2->left->parent= p1;
					p2->right->parent= n;
					p1->right= p2->left;
					p2->left= p1;
					n->left= p2->right;
					p2->right= n;
					n->balanced= (p2->balanced < 0);
					p1->balanced= (p2->balanced > 0) ? -1 : 0;
					*nn= p2;
				}
				n->balanced= 0;
				*balanced= 0;
			}
		}
		return r;
	} else if (mut_exec_addr_lt(n->addr, addr)) {
		r= mut_resource_internal_add(&n->right, addr, balanced, n);
		if (r == 0)
			return r;
		if (*balanced) {	/* right branch has grown longer */
			int n_balanced= n->balanced;
			if (n_balanced < 0) {
				n->balanced= 0;
				*balanced= 0;
			} else if (n_balanced == 0) {
				n->balanced= 1;
			} else /* n_balanced > 0 */ {
				p1= n->right;
				if (p1->balanced > 0) {	/* RR */
					n->right= p1->left;
					p1->left= n;
					n->balanced= 0;
					*nn= p1;
				} else {		/* double RL */
					p2= p1->left;
					p1->left= p2->right;
					p2->right= p1;
					n->right= p2->left;
					p2->left= n;
					n->balanced= (p2->balanced > 0) ? -1 : 0;
					p1->balanced= (p2->balanced < 0);
					*nn= p2;
				}
				n->balanced= 0;
				*balanced= 0;
			}
		}
		return r;
	} else {
		*balanced= 0;
		return n;
	}
}



mut_resource *mut_resources_create_resource(mut_resources *rs, mut_exec_addr addr)
{
	mut_resource * r= mut_resources_internal_add(&rs->elems, addr);
	if (mut_resource_is_new(r)) {
		rs->n_active_elems += 1;
	}
	return r;
}



static mut_resource *mut_resources_internal_lookup(mut_resource *r, mut_exec_addr *addr)
{
	if (r != 0) {
		if (mut_exec_addr_lt(addr, r->addr))
			return mut_resources_internal_lookup(r->left, addr);
		if (mut_exec_addr_lt(r->addr, addr))
			return mut_resources_internal_lookup(r->right, addr);
	}
	return r;
}



mut_resource *mut_resources_lookup(mut_resources *rs, mut_exec_addr addr)
{
	return mut_resources_internal_lookup(rs->elems, addr);
}


size_t mut_resources_iter_open(mut_resources *rs, mut_resources_iter *iter, int all)
{
	iter->elem= rs->elems;
	iter->all= all;
	return rs->n_active_elems + (all ? rs->n_consumed_elems : 0);
}



mut_resource *mut_resources_iter_read(mut_resources_iter *iter)
{
	int all= iter->all;
	mut_resource *item;
next:
	item= iter->elem;
	if (item == (mut_resource *)0)
		return item;
	if (direction < 0) {
		if (item->left) {
			if (item->left) {
				iter->elem= item->left;
			} else if (item->right) {
				if (!all && (item->consumer.action != (mut_action *)0))
					goto next;
			}
			return item;
		}
	}
}


void mut_resources_iter_close(mut_resources_iter *iter)
{
	iter->elem= (mut_resource *)0;
}



static void mut_resources_internal_close(mut_resource *r)
{
	if (r->left) 
		mut_resources_internal_close(r->left);
	if (r->right) 
		mut_resources_internal_close(r->right);
	if (r->producer.backtrace)
		mut_backtrace_close(r->producer.backtrace);
	if (r->consumer.backtrace)
		mut_backtrace_close(r->consumer.backtrace);
	mut_mem_free(r);
}


void mut_resources_close(mut_resources *rs)
{
	if (rs->elems)
		mut_resources_internal_close(rs->elems);
}

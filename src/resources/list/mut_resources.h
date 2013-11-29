#ifndef MUT_RESOURCES_H
#define MUT_RESOURCES_H

/*
 *.intro: linked list implementation of mut_resources
 *
 */

struct mut_resource {
	struct mut_resource * next;
	mut_exec_addr         addr;
	size_t                size;
	struct {
		mut_action    * action;
		mut_backtrace * backtrace;
	} producer;
	struct {
		mut_action    * action;
		mut_backtrace * backtrace;
	} consumer;
};

typedef struct mut_resource mut_resource;

#define mut_resource_is_new(obj) ((obj)->producer.backtrace == 0)
#define mut_resource_producer_name(obj) ((obj)->producer.action->name)
#define mut_resource_consumer_name(obj) ((obj)->consumer.action->name)

void mut_resource_make_producer(mut_resource *, mut_action *, size_t, mut_backtrace *);


struct mut_resources {
	mut_resource *elems;
	mut_log *log;
	void (*delete)(struct mut_resources *, mut_resource *, mut_action *, mut_backtrace *);
	size_t n_active_elems;
	size_t n_consumed_elems;
};

typedef struct mut_resources mut_resources;

struct mut_resources_iter {
	mut_resource * elem;
	int            all;
};

typedef struct mut_resources_iter mut_resources_iter;

void mut_resources_open(mut_resources *, mut_log *, int);

mut_resource *mut_resources_create_resource(mut_resources *, mut_exec_addr);

mut_resource *mut_resources_lookup(mut_resources *, mut_exec_addr);

#define mut_resources_delete_resource(rs, obj, consumer, bt) \
     do {                                                    \
          mut_resources * _map= (rs);                        \
          _map->delete(_map, obj, consumer, bt);             \
     } while (0)

#define mut_resources_resource_deleted(rs, obj) ((obj)->consumer.action != 0)

size_t mut_resources_iter_open(mut_resources *, mut_resources_iter *, int);

mut_resource *mut_resources_iter_read(mut_resources_iter *);

void mut_resources_iter_close(mut_resources_iter *);

void mut_resources_close(mut_resources *);

#endif

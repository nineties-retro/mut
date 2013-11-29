#ifndef MUT_H
#define MUT_H

#include "mut_exec_addr.h"
#include "mut_action.h"
#include "mut_backtrace_sharer.h"
#include "mut_resources.h"
#include "mut_exec.h"
#include "mut_process_ref.h"

enum mut_control {
	mut_control_trace=     (1<<0),
	mut_control_backtrace= (1<<1),
	mut_control_free=      (1<<2),
	mut_control_stats=     (1<<3),
	mut_control_zero=      (1<<4),
	mut_control_usage=     (1<<5),
	mut_control_check=     (1<<6)
};


typedef enum mut_control mut_control;

struct mut_stat {
	mut_counter space;
	mut_counter count;
};

struct mut {
	mut_log     * log;
	mut_process   process;
	mut_exec      exec;
	struct {
		mut_action    malloc;
		mut_action    calloc;
		mut_action    realloc;
		mut_action    free;
		mut_action    cfree;
		mut_action    usage;
		mut_action  * root;
	} actions;
	mut_resources resources;
	mut_control   flags;
	struct {
		struct mut_stat now;
		struct mut_stat total;
		struct {
			struct mut_stat space;
			struct mut_stat count;
		} max;
	} stats;
};

typedef struct mut mut;

int mut_open(mut *, char **, mut_log *, mut_control, char const *, const char *);

int mut_run(mut *);

int mut_close(mut *);

void mut_close_on_error(mut *);

#endif

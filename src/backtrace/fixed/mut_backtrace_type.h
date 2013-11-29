#ifndef MUT_BACKTRACE_TYPE_H
#define MUT_BACKTRACE_TYPE_H

/*
 * An arbitrary limit on the number of entries in a backtrace.
 * Larger would obviously be better but it wastes memory.
 */
#define mut_backtrace_frames_max 20

struct mut_backtrace {
	size_t        ref_count;
	size_t        n_frames;
	struct {
		mut_exec_addr return_addr;
	} frame[mut_backtrace_frames_max];
	mut_exec * exec;
};

typedef struct mut_backtrace mut_backtrace;

#endif

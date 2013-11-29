#ifndef MUT_LOG_TYPE_H
#define MUT_LOG_TYPE_H

#include "mut_stdio.h"

/*
 *.fatal_error: true if the error is a fatal error 
 *.internal_error: true if there has been an internal error logging
 * the message.
 *.logging: just here to help catch simple errors using the mut_log interface.
 */
struct mut_log {
	char const * program_name;
	int          fatal_error;
	int          internal_error;
	int          logging;
	FILE       * sink;
};

typedef struct mut_log mut_log;

#endif

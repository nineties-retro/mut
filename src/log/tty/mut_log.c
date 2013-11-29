/*
 * $Id$
 */

#include "mut_assert.h"		/* mut_assert_pre */
#include "mut_stddef.h"		/* size_t */
#include "mut_stdio.h"		/* fputs, stderr */
#include "mut_log.h"
#include "mut_log_own.h"
#include "mut_uint64.h"
#include "mut_uint64_out.h"
#include "mut_log_uint64.h"

void mut_log_open(mut_log *log, char const * program_name)
{
	log->fatal_error = 0;
	log->internal_error =  0;
	log->program_name = program_name;
	log->logging = 0;
	log->sink = stderr;
}



void mut_log_prog_name(mut_log *log, char const * prog_name)
{
	log->program_name = prog_name;
}



void mut_log_fatal(mut_log *log, char const * fatal, int error_code)
{
	mut_assert_pre(!log->fatal_error);
	mut_assert_pre(!log->logging);
	mut_assert_pre(!log->internal_error);

	log->fatal_error = 1;
	log->logging = 1;
	log->internal_error = fprintf(log->sink, "%s fatal %d %s", log->program_name, error_code, fatal) < 0;
}


void mut_log_mem_full(mut_log *log, int error_code)
{
	mut_assert_pre(!log->fatal_error);
	mut_assert_pre(!log->logging);
	mut_assert_pre(!log->internal_error);

	log->fatal_error = 1;
	log->logging = 1;
	log->internal_error = fprintf(log->sink, "%s fatal %d mem.full\n", log->program_name, error_code) < 0;
}



void mut_log_error(mut_log *log, char const *error)
{
	mut_assert_pre(!log->fatal_error);
	mut_assert_pre(!log->logging);
	mut_assert_pre(!log->internal_error);

	log->logging = 1;
	log->internal_error = fprintf(log->sink, "%s error %s", log->program_name, error) < 0;
}



void mut_log_warning(mut_log *log, char const * warning)
{
	mut_assert_pre(!log->fatal_error);
	mut_assert_pre(!log->logging);
	mut_assert_pre(!log->internal_error);
	
	log->logging = 1;
	log->internal_error = fprintf(log->sink, "%s warning %s", log->program_name, warning) < 0;
}


void mut_log_info(mut_log *log, char const * info)
{
	mut_assert_pre(!log->fatal_error);
	mut_assert_pre(!log->logging);
	mut_assert_pre(!log->internal_error);
	
	log->logging = 1;
	log->internal_error = fprintf(log->sink, "%s info %s", log->program_name, info) < 0;
}



void mut_log_string(mut_log *log, char const * arg)
{
	mut_assert_pre(log->logging);

	if (!log->internal_error)
		log->internal_error = fprintf(log->sink, " %s", arg) < 0;
}


void mut_log_int(mut_log *log, int i)
{
	mut_assert_pre(log->logging);

	if (!log->internal_error)
		log->internal_error = fprintf(log->sink, " %d", i) < 0;
}


void mut_log_uint(mut_log *log, unsigned int i)
{
	mut_assert_pre(log->logging);

	if (!log->internal_error)
		log->internal_error = fprintf(log->sink, " %u", i) < 0;
}


void mut_log_uint64(mut_log *log, mut_uint64 i)
{
	mut_assert_pre(log->logging);

	if (!log->internal_error)
		log->internal_error = fputc(' ', log->sink) == EOF 
			|| !mut_uint64_out(log->sink, i);
}


void mut_log_uint_hex(mut_log *log, unsigned int i)
{
	mut_assert_pre(log->logging);

	if (!log->internal_error)
		log->internal_error = fprintf(log->sink, " %X", i) < 0;
}



void mut_log_long(mut_log *log, long i)
{
	mut_assert_pre(log->logging);

	if (!log->internal_error)
		log->internal_error = fprintf(log->sink, " %ld", i) < 0;
}


void mut_log_ulong(mut_log *log, unsigned long i)
{
	mut_assert_pre(log->logging);

	if (!log->internal_error)
		log->internal_error = fprintf(log->sink, " %lu", i) < 0;
}


int mut_log_end(mut_log *log)
{
	mut_assert_pre(log->logging);
	log->logging= 0;
	if (!log->internal_error)
		log->internal_error= fputs("\n", log->sink) < 0;
	return !log->internal_error;
}


void mut_log_file(mut_log *log, char const *log_file_name)
{
	FILE *sink;

	if ((sink= fopen(log_file_name, "w")) == (FILE *)0) {
		mut_log_warning(log, "io.open");
		mut_log_string(log, log_file_name);
		mut_log_end(log);
	} else {
		log->sink = sink;
	}
}


int mut_log_close(mut_log *log)
{
	if (log->sink != stderr && log->sink != stdout) {
		(void)fclose(log->sink);
	}
	return log->internal_error;
}

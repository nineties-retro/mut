/*
 *.intro: command line interface.
 */

#include "mut_stdlib.h"		/* EXIT_XXX */
#include "mut_stdio.h"		/* FILE * */
#include "mut_mem_ctrl.h"
#include "mut_log_own.h"
#include "mut_log.h"
#include "mut.h"

struct mut_ui_arg_info {
	mut_log      * log;
	char        ** argv;
	char const   * log_file_name;
	char const   * mut_name;
	mut_control    flags;
	char const   * undesirable_suffix;
	const char   * usage_function;
};

typedef struct mut_ui_arg_info mut_ui_arg_info;


static void mut_ui_usage(mut_log * log)
{
	mut_log_fatal(log, "args.usage", 0);
	mut_log_string(log, "exec-file-name");
	(void)mut_log_end(log);
}


/*
 * XREF: <URI:mut/README#bad.getopt>
 */
static int mut_ui_process_args(mut_ui_arg_info *info, int argc, char **argv)
{
options_loop:
	argv += 1;
	argc -= 1;
	if (argc == 0) {
		mut_ui_usage(info->log);
		return 0;
	} else if (argv[0][0] != '-') {
		info->argv= argv;
		return 1;
	} else {
		char const *arg= &argv[0][0];
	option_loop:
		arg += 1;
		switch(*arg) {
		case '\0':
			goto options_loop;
		case 'r':
			mut_mem_report(stderr);
			break;
		case 'f':			/* XREF: <URI:mut/doc/mut.1#f> */
			info->flags |= mut_control_free;
			break;
		case 't':			/* XREF: <URI:mut/doc/mut.1#t> */
			info->flags |= mut_control_trace;
			break;
		case 'b':			/* XREF: <URI:mut/doc/mut.1#b> */
			info->flags |= mut_control_backtrace;
			break;
		case 'c':			/* XREF: <URI:mut/doc/mut.1#c> */
			info->flags |= mut_control_check;
			break;
		case 's':			/* XREF: <URI:mut/doc/mut.1#s> */
			info->flags |= mut_control_stats;
			break;
		case 'u':			/* XREF: <URI:mut/doc/mut.1#u> */
			info->flags |= mut_control_usage;
			break;
		case 'o':			/* XREF: <URI:mut/doc/mut.1#o> */
			if (arg == &argv[0][1]) {
				info->log_file_name = &argv[0][2];
			} else {
				char opt[2];
				opt[0] = *arg;
				opt[1] = '\0';
				mut_log_fatal(info->log, "args.opt", 0);
				mut_log_string(info->log, opt);
				mut_log_string(info->log, argv[0]);
				(void)mut_log_end(info->log);
				return 0;
			}
			goto options_loop;
		case 'x':			/* XREF: <URI:mut/doc/mut.1#x> */
			if (arg == &argv[0][1]) {
				info->undesirable_suffix = &argv[0][2];
			} else {
				char opt[2];
				opt[0] = *arg;
				opt[1] = '\0';
				mut_log_fatal(info->log, "args.opt", 0);
				mut_log_string(info->log, opt);
				mut_log_string(info->log, argv[0]);
				(void)mut_log_end(info->log);
				return 0;
			}
			goto options_loop;
		case 'n':			/* XREF: <URI:mut/doc/mut.1#n> */
			if (arg == &argv[0][1]) {
				info->mut_name = &argv[0][2];
			} else {
				char opt[2];
				opt[0] = *arg;
				opt[1] = '\0';
				mut_log_fatal(info->log, "args.opt", 0);
				mut_log_string(info->log, opt);
				mut_log_string(info->log, argv[0]);
				(void)mut_log_end(info->log);
				return 0;
			}
			goto options_loop;
		case 'U':			/* XREF: <URI:mut/doc/mut.1#U> */
			if (arg == &argv[0][1]) {
				info->usage_function = &argv[0][2];
			} else {
				char opt[2];
				opt[0] = *arg;
				opt[1] = '\0';
				mut_log_fatal(info->log, "args.opt", 0);
				mut_log_string(info->log, opt);
				mut_log_string(info->log, argv[0]);
				(void)mut_log_end(info->log);
				return 0;
			}
			goto options_loop;
		case 'z':
			/* XREF: <URI:mut/doc/mut.1#z>
			 * XREF: <URI:mut/tests/afm_free.c>
			 */
			info->flags |= mut_control_zero;
			break;
		default:
			mut_log_fatal(info->log, "args.unknown", 0);
			mut_log_string(info->log, argv[0]);
			(void)mut_log_end(info->log);
			return 0;
		}
		goto option_loop;
	}
}



int main(int argc, char ** argv)
{
	mut             manager;
	mut_log         log;
	mut_ui_arg_info info;

	mut_mem_init();
	mut_mem_open();
	mut_log_open(&log, argv[0]);

	info.log = &log;
	info.flags = 0;
	info.log_file_name = (char *)0;
	info.mut_name = (char *)0;
	info.undesirable_suffix = "@@GLIBC_2.0";
	info.usage_function = 0;

	if (!mut_ui_process_args(&info, argc, argv))
		goto could_not_process_arguments;

	if (info.log_file_name != (char *)0)
		mut_log_file(&log, info.log_file_name);

	if (info.mut_name != (char *)0)
		mut_log_prog_name(&log, info.mut_name);

	if (!mut_open(&manager, info.argv, info.log, 
		      info.flags, info.undesirable_suffix, info.usage_function))
		goto could_not_open_manager;

	if (!mut_run(&manager))
		goto could_not_run;

	if (!mut_close(&manager))
		goto could_not_close_manager;

	mut_log_close(&log);
	mut_mem_close();
	return EXIT_SUCCESS;

could_not_run:
	mut_close_on_error(&manager);
could_not_close_manager:
could_not_open_manager:
could_not_process_arguments:
	mut_log_close(&log);
	mut_mem_close();
	return EXIT_FAILURE;
}

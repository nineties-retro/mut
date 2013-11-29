/*
 *.intro: tests that CPU limits are supported.
 *
 * If given an argument, sets the CPU limit to that many seconds and
 * then runs an infinite loop.
 */

#include <sys/time.h>		/* setrlimit */
#include <sys/resource.h>	/* setrlimit */
#include <unistd.h>		/* setrlimit */
#include <stdio.h>		/* fprintf, stderr */
#include <string.h>		/* strerror */
#include <errno.h>		/* errno */
#include <stdlib.h>		/* atoi */

int main(int argc, char **argv)
{
	if (argc > 1) {
		struct rlimit cpu;
		cpu.rlim_cur= atoi(argv[1]);
		cpu.rlim_max= atoi(argv[1]);
		if (setrlimit(RLIMIT_CPU, &cpu) < 0) {
			fprintf(stderr, "%s: could not set limit: %s\n", argv[0], strerror(errno));
			return 0;
		}
	}
	for (;;) {
		/* do nothing */
	}
	return 0;
}

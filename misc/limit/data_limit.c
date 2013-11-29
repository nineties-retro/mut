/*
 *.intro: tests that DATA limits are supported.
 *
 * If given an argument, sets the DATA limit to that many seconds and
 * then continuously mallocs 64K chunks until memory runs out.
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
		struct rlimit data;
		data.rlim_cur= atoi(argv[1]);
		data.rlim_max= atoi(argv[1]);
		if (setrlimit(RLIMIT_DATA, &data) < 0) {
			fprintf(stderr, "%s: could not set limit: %s\n", argv[0], strerror(errno));
			return 0;
		}
	}
	for (;;) {
		char *x= malloc(1<<16);
		if (x == (char *)0) {
			write(1, "Z\n", 2);
			return 1;
		} else {
			write(1, "X", 1);
		}
	}
	return 0;
}

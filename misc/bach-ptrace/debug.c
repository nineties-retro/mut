#include <unistd.h>		/* fork, pid_t */
#include <sys/ptrace.h>		/* ptrace, PTRACE_TRACEME, ... etc. */
#include <sys/wait.h>		/* wait */
#include <stdio.h>

#define TR_SETUP PTRACE_TRACEME
#define TR_WRITE PTRACE_POKEDATA
#define TR_RESUME PTRACE_CONT

long addr;

int main(int argc, char **argv)
{
	int i;
	pid_t pid;

	sscanf(argv[1], "%lx", &addr);

	if ((pid= fork()) == 0) {
		ptrace(TR_SETUP, 0, 0, 0);
		execl("trace", "trace", 0);
		exit(0);
	}
	wait((int *)0);
	for (i= 0; i<32; i++) {
		/* write value of i into address addr in proc pid */
		if (ptrace(TR_WRITE, pid, addr, i) == -1)
			exit(1);
		addr += sizeof(int);
	}
	/* traced process should resume execution */
	ptrace(TR_RESUME, pid, 1, 0);
	return 0;
}

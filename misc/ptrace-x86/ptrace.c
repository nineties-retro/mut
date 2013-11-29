/*
 *.intro: A simple ptrace test for an x86.
 *
 * This program was thrown together in order to try out ptrace.
 * It should not be take as an example of good style.
 */

#include <stdio.h>		/* fprintf, sprintf */
#include <stdlib.h>		/* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/types.h>		/* waitpid, open */
#include <fcntl.h>		/* O_RDONLY */
#include <unistd.h>		/* fork, pid_t, open */
#include <signal.h>		/* kill, SIGTERM */
#include <assert.h>		/* assert */
#include <sys/ptrace.h>		/* ptrace, PTRACE_TRACEME, EIP */
#include <sys/wait.h>		/* waitpid */

#define reg_offset(reg_num) ((reg_num)*4)


static void usage(char const * program_name)
{
	fprintf(stderr, "%s: executable\n", program_name);
}



static void kill_child(pid_t pid)
{
	kill(pid, SIGTERM);
}


static void do_child(char const *program_name, char **argv)
{
	assert(program_name != (char *)0);
	assert(argv[0] != (char *)0);

	if (ptrace(PTRACE_TRACEME, (pid_t)0, 0, 0) < 0) {
		fprintf(stderr, "%s: could not trace %s due to %s\n", program_name, argv[0], strerror(errno));
		exit(EXIT_FAILURE);
	}
	execv(argv[0], argv);
	fprintf(stderr, "%s: could not exec %s due to %s\n", program_name, argv[0], strerror(errno));
	exit(EXIT_FAILURE);
}



static int do_parent(char const * program_name, 
		     char const * executable_name,
		     pid_t        child_pid)
{
	int child_status;
	long pc;

	puts("Parent: waiting for child ... ");
	if (waitpid(child_pid, &child_status, 0) < 0) {
		fprintf(stderr, "%s: could not wait for %s due to %s\n",
			program_name, executable_name, strerror(errno));
		goto could_not_wait;
	}

	printf("Extracting EIP ...\n");
	if ((pc= ptrace(PTRACE_PEEKUSR, child_pid, reg_offset(EIP), 0)) < 0) {
		fprintf(stderr, "%s: could not determine PC due to %s\n", program_name, strerror(errno));
		goto could_not_determine_pc;
	}
  
	printf("EIP= %lx, instr= %x\n", pc, ptrace(PTRACE_PEEKTEXT, child_pid, pc, 0));
	printf("Continuing at %lx...\n", pc);
	if (ptrace(PTRACE_CONT, child_pid, 0, 0) < 0) {
		fprintf(stderr, "%s: could not continue %s due to %s\n",
			program_name, executable_name, strerror(errno));
		goto could_not_continue;
	}
	puts("Parent: waiting for child ... ");
	if (waitpid(child_pid, &child_status, 0) < 0) {
		fprintf(stderr, "%s: could not wait for %s due to %s\n",
			program_name, executable_name, strerror(errno));
		goto could_not_wait;
	}
	printf("Extracting EIP ...\n");
	if ((pc= ptrace(PTRACE_PEEKUSR, child_pid, reg_offset(EIP), 0)) < 0) {
		fprintf(stderr, "%s: could not determine PC due to %s\n", program_name, strerror(errno));
		goto could_not_determine_pc;
	}
	printf("EIP= %lx, instr= %x\n", pc, ptrace(PTRACE_PEEKTEXT, child_pid, pc, 0));
	return EXIT_SUCCESS;

could_not_continue:
could_not_determine_pc:
could_not_wait:
	kill_child(child_pid);
	return EXIT_FAILURE;
}


static int debug(char const * program_name, char ** argv)
{
	pid_t pid= fork();
	if (pid < 0) {
		fprintf(stderr, "%s: could not fork due to %s\n", program_name, strerror(errno));
		return EXIT_FAILURE;
	} else if (pid == 0) {
		do_child(program_name, argv);
		return EXIT_FAILURE;	/* keep dump compilers happy */
	} else {
		return do_parent(program_name, argv[0], pid);
	}
}



int main(int argc, char **argv)
{
	char const * program_name= argv[0];
	while (++argv, --argc != 0 && argv[0][0] == '-') {
		switch(argv[0][1]) {
		default:
			usage(program_name);
			return EXIT_FAILURE;
		}
	}
	if (argc != 1) {
		usage(program_name);
		return EXIT_FAILURE;
	}
	return debug(program_name, argv);
}

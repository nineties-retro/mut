/*
 *.intro: /proc based debugging.
 */


#include <fcntl.h>		/* open */
#include <sys/types.h>		/* pid_t */
#include <unistd.h>		/* fork, close */
#include <sys/syscall.h>	/* SYS_execve */
#include <sys/ioctl.h>		/* ioctl */
#include "mut_stdlib.h"		/* exit, EXIT_FAILURE */
#include "mut_stdio.h"		/* sprintf */
#include "mut_assert.h"		/* mut_assert_pre */
#include "mut_errno.h"		/* errno */
#include "mut_log.h"		/* mut_log */
#include "mut_exec.h"		/* mut_exec_addr */
#include "mut_instr.h"		/* mut_instr_breakpoint */
#include "mut_process.h"

int mut_process_child(mut_process *p, char ** argv, mut_log *log)
{
	sysset_t exit_set;
	pid_t child_pid = getpid();
	char proc_file_name[6];
	int fd;

	sprintf(proc_file_name, "%d", child_pid);
	if ((fd= open(proc_file_name, O_RDWR)) < 0) {
		mut_log_fatal(log, "io.open", errno);
		mut_log_string(log, proc_file_name);
		(void)mut_log_end(log);
		goto could_not_open_proc_file;
	}
	premptyset(&exit_set);
	praddset(&exit_set, SYS_execve);
	if (ioctl(fd, PIOCSEXIT, &exit_set) < 0) {
		mut_log_fatal(log, "ioctl", errno);
		mut_log_string(log, proc_file_name);
		(void)mut_log_end(log);
		goto could_not_set_exit_set;
	}
	if (close(fd) < 0) {
		mut_log_warning(log, "io.close", errno);
		mut_log_string(log, proc_file_name);
		(void)mut_log_end(log);
	}
	execv(argv[0], argv);
	mut_log_fatal(log, "execv", errno);
	mut_log_string(log, argv[0]);
	(void)mut_log_end(log);
	_exit(EXIT_FAILURE);

could_not_set_exit_set:
	close(fd);
could_not_open_proc_file:
	_exit(EXIT_FAILURE);
}



static int mut_process_parent(mut_process *p, pid_t child_pid, mut_log *log)
{
	char proc_file_name[6];
	int fd;

	sprintf(proc_file_name, "%d", child_pid);
	if ((p->fd= open(proc_file_name, O_RDWR)) < 0) {
		mut_log_fatal(log, "io.open", errno);
		mut_log_string(log, proc_file_name);
		(void)mut_log_end(log);
		goto could_not_open_proc_file;
	}
	p->status= mut_process_status_stopped;
	p->log= log;
	return 1;

could_not_open_proc_file:
	return 0;
}



int mut_process_open(mut_process *p, char **argv, mut_log *log)
{
	pid_t pid = fork();

	if (pid < 0) {
		mut_log_fatal(log, "fork", errno);
		mut_log_string(log, argv[0]);
		(void)mut_log_end(log);
		return 0;
	} if (pid == 0) {
		return mut_process_child(p, argv, log);
	} else {
		return mut_process_parent(p, pid, log);
	}
}



int mut_process_wait(mut_process *p, mut_exec_addr addr)
{
	prstatus_t status;

	if (ioctl(p->fd, PIOCWSTOP, &status) < 0) {
		mut_log_fatal(log, "ioctl", errno);
		(void)mut_log_end(log);
		goto could_not_wait;
	}
	printf("mut_process_wait: flags=%x\n", status.pr_flags);
	printf("mut_process_wait: why=%x\n", status.pr_why);
	p->pc = status.pr_reg[EIP];
	*addr = p->pc;
	return 1;

could_not_wait:
	return 0;
}



int mut_process_breakpoint(mut_process *p, mut_exec_addr pc, mut_instr *instr)
{
	mut_assert_pre(p->stopped);

	printf("mut_process_breakpoint: "mut_exec_addr_fmt"\n", pc);

	return 1;
}



int mut_process_restore(mut_process *p, mut_exec_addr pc, mut_instr instr)
{
	mut_assert_pre(p->stopped);

	printf("mut_process_restore: "mut_exec_addr_fmt"\n", pc);
	return 1;
}


int mut_process_resume(mut_process *p)
{
	mut_assert_pre(p->stopped);

	printf("mut_process_resume:@"mut_exec_addr_fmt"\n", p->pc);
	p->pc= 0;
	p->stopped= 0;
	return 1;
}



void mut_process_close(mut_process *p)
{
	mut_assert_pre(p->stopped);
	if (close(p->fd) < 0) {
		mut_log_warning(log, "io.close", errno);
		(void)mut_log_end(log);
	}
}

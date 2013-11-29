#ifndef MUT_MEM_CTRL_H
#define MUT_MEM_CTRL_H

void   mut_mem_init(void);
void   mut_mem_open(void);
FILE * mut_mem_log(FILE *);
FILE * mut_mem_report(FILE *);
int    mut_mem_close(void);

#endif

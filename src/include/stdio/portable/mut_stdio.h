#ifndef MUT_STDIO_H
#define MUT_STDIO_H

#include <stdio.h>

extern int      fclose(FILE *);
extern int      fprintf(FILE *, const char *, ...);
extern int      fputs(const char *, FILE *);
extern size_t   fread(void *, size_t, size_t, FILE *);
extern int      fseek(FILE *, long, int);

#define SEEK_SET 0

#endif

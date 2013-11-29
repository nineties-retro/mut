#ifndef MUT_MEM_H
#define MUT_MEM_H

/*
 * Include this instead of a <stdlib.h> anywhere <stdlib.h> was being used
 * to pull in malloc/realloc/free
 */

void * mut_mem__malloc(size_t, size_t, char const *);
void * mut_mem__realloc(void *, size_t, size_t, char const *);
void mut_mem__free(void *, size_t, char const *);

#define mut_mem_malloc(n_bytes) mut_mem__malloc(n_bytes, __LINE__, __FILE__)

#define mut_mem_realloc(addr, n_bytes) mut_mem__realloc(addr, n_bytes, __LINE__, __FILE__)

#define mut_mem_free(addr)  mut_mem__free(addr, __LINE__, __FILE__)

#endif

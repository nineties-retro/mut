/*
 *.intro: test for a realloc which is defined in terms of malloc&free
 *
 * Note the following doesn't implement a real malloc, realloc, free, it
 * just has enough for testing purposes.
 */
typedef unsigned int size_t;

extern int write(int, const void *, size_t);

static char buffer[4096];
static char *p= &buffer[0];

static void *malloc(size_t nbytes)
{
	if (nbytes < (&buffer[4096] - p)) {
		void *r= p;
		p += nbytes;
		return r;
	} else {
		return 0;
	}
}

static void free(void *p)
{
}


static void *realloc(void *p, size_t nbytes)
{
	if (p == 0) {
		return malloc(nbytes);
	} else if (nbytes == 0) {
		free(p);
	}
	return 0;
}




int main(void)
{
	char *x;
	write(1, "A\n", 2);
	x = realloc(0, 8);
	write(1, "B\n", 2);
	free(x);
	write(1, "C\n", 2);
	return 0;
}

/*
 *.intro: what if malloc is not a function?
 *
 * The idea is to ensure that an attempt isn't made to fiddle with a symbol
 * unless it is a function.  Unfortunately doesn't currently work.
 * See <URI:mut/README#status.bogus-definitions>.
 */

int malloc;

typedef int realloc;

int main(void)
{
	return 0;
}

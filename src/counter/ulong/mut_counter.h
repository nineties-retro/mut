#ifndef MUT_COUNTER_H
#define MUT_COUNTER_H

typedef unsigned long mut_counter;

#define mut_counter_from_uint(ui) (ui)
#define mut_counter_inc(c, d) ((c)+(d))
#define mut_counter_dec(c, d) ((c)-(d))
#define mut_counter_lt(a, b) ((a) < (b))

#endif

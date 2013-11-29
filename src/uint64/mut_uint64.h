			     -*- Text -*-

.intro: template for mut_uint64.h

typedef ... mut_uint64;

extern mut_uint64
mut_uint64_from_uint(unsigned int n);

  `mut_uint64_from_uint' converts an unsigned integer `n' to an
  unsigned 64-bit value.  This is useful for initialising a 64-bit
  value. 


extern mut_uint64
mut_uint64_inc(mut_uint64 ui, unsigned int delta);

  `mut_uint64_inc' increments `ui' by `delta'.  It is undefined
  whether overflow is detected and/or reported.  XXX: this is because
  it is unlikely to occur, neverthless, a consistent story needs to be
  constructed in this case.


extern mut_uint64
mut_uint64_dec(mut_uint64 ui, unsigned int delta);

  `mut_uint64_dec' decrements `ui' by `delta'.  It is undefined
  whether underflow is detected and/or reported.  XXX: this is because
  it is unlikely to occur, neverthless, a consistent story needs to be
  constructed in this case.


extern int
mut_uint64_lt(mut_uint64 a, mut_uint64 b);

  `mut_uint64_lt' returns a non-zero value if a < b.

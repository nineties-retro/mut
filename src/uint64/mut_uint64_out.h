			     -*- Text -*-

.intro: template for mut_uint64_out.h

extern int mut_uint64_out(FILE * output, mut_uint64 n);

  `mut_uint64_out' writes `n' to `output' in decimal with no leading
  spaces or zeros.

  `mut_uint64_out' returns a non-zero value on success and zero if the
  value could not be output correctly.

// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
// XFAIL: *

// This is an example, that CANNOT be compiled due to absence of ADRF moves.
void main(int dest, tensor in) {
  int5 c0 = 0;
  int __global *ptr = (__global int *) a_gen_addr_i(c0, in);
  c0[0]++;
  while(1) {
    int __global *ptr_w = (__global int *) a_gen_addr_i(c0, in);
    if (*ptr_w > *ptr)
      ptr = (__global int *) a_gen_addr_i(c0, in);  // expected-error{{put actual message here}}
    else
      break;
  }
  *(int __local *)dest = *ptr;
}
// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
// XFAIL: *

// This is an example, that CANNOT be compiled due to absence of ADRF moves.
void main(int dest, tensor in) {
  int5 c0 = 0;
  int __global *ptr = (__global int *) gen_addr(c0, in, 0, 0, 1, 0);
  c0[0]++;
  while(1) {
    int __global *ptr_w = (__global int *) gen_addr(c0, in, 0, 0, 1, 0);
    if (*ptr_w > *ptr)
      ptr = (__global int *) gen_addr(c0, in, 0, 0, 1, 0);  // expected-error{{put actual message here}}
    else
      break;
  }
  *(int __local *)dest = *ptr;
}
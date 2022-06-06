// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

void main(int dest, tensor out) {
  int __global *ptr, *ptr1;
  int5 c0 = 0;
  ptr = (__global int *) gen_addr(c0, out, 0, 0, 1, 0);
  c0[0] = 1;
  ptr1 = (__global int *) gen_addr(c0, out, 0, 0, 1, 0);
  int *dptr = (int __local *)dest;

  dptr[0] = ptr == ptr1;  // expected-error{{unsupported operation on global pointer}}
  dptr[1] = ptr != ptr1;  // expected-error{{unsupported operation on global pointer}}
  dptr[2] = ptr <= ptr1;  // expected-error{{unsupported operation on global pointer}}
  dptr[3] = ptr >= ptr1;  // expected-error{{unsupported operation on global pointer}}
  dptr[4] = ptr < ptr1;   // expected-error{{unsupported operation on global pointer}}
  dptr[5] = ptr > ptr1;   // expected-error{{unsupported operation on global pointer}}
}
// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

void main(int x1, tensor out) {
  int __global *ptr, *ptr1;
  int5 c0 = 0;
  ptr = (__global int *) a_gen_addr_i(c0, out);
  ptr1 = (__global int *) a_gen_addr_i(c0, out);

  ptr = (__global int *)12; // expected-error{{unsupported operation on global pointer}}
  ptr[x1] = 1;   // expected-error{{unsupported operation on global pointer}}
  x1[ptr] = 1;   // expected-error{{unsupported operation on global pointer}}
  ptr = ptr1;    // expected-error{{unsupported operation on global pointer}}
  if (!ptr)      // expected-error{{unsupported operation on global pointer}}
    *ptr1 = 12;
}
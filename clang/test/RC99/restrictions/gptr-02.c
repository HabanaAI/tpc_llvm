// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

void main(int x1, tensor out) {
  int __global *ptr, *ptr1;
  int5 c0 = 0;
  ptr = (__global int *) gen_addr(c0, out, 0, 0, 1, 0);
  ptr1 = (__global int *) gen_addr(c0, out, 0, 0, 1, 0);

  *(ptr + 2) = 2;      // expected-error{{unsupported operation on global pointer}}
  *(2 + ptr) = 2;      // expected-error{{unsupported operation on global pointer}}
  ptr += 2;            // expected-error{{unsupported operation on global pointer}}
  *(ptr - 2) = 3;      // expected-error{{unsupported operation on global pointer}}
  ptr -= 2;            // expected-error{{unsupported operation on global pointer}}
  *ptr = (ptr - ptr1); // expected-error{{unsupported operation on global pointer}}
}
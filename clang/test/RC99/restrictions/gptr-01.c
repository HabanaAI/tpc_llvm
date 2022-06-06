// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

void main(int x1, tensor out) {
  int __global *ptr;
  int5 c0 = 0;
  ptr = (__global int *) gen_addr(c0, out, 0, 0, 1, 0);

  ptr++; // expected-error{{unsupported operation on global pointer}}
  ++ptr; // expected-error{{unsupported operation on global pointer}}
  ptr--; // expected-error{{unsupported operation on global pointer}}
  --ptr; // expected-error{{unsupported operation on global pointer}}
}
// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

struct ABC {
  int f1;
  int f2;
};

struct BCD {
  int f1;
  int __global *f2;  // expected-error{{fields cannot be global pointers}}
};

void main(int x1, tensor out) {
  struct ABC __global *ptr;     // expected-error{{global pointers may point to integer, floating or vector types only}}
  int __global * __local *ptr1; // expected-error{{pointers to global pointers are not allowed}}
  int __global *arr[10];        // expected-error{{arrays of global pointers are not allowed}}
  struct BCD arr2[10];
}
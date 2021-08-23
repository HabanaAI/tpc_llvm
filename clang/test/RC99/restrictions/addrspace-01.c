// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

__global__ int a;      // expected-error{{variables in global address space are not allowed}}
__global__ int *ptr_a = &a;  // expected-error{{global or static variables of global pointer type are not allowed}}

int b;
int *ptr_b = &b;

__local__ int c;
__local__ int *ptr_c = &c;

void main() {
  ptr_b = &c;
  ptr_c = &b;
  ptr_b = ptr_c;
  static __global__ int *ptr_a = &a;  // expected-error{{global or static variables of global pointer type are not allowed}}


  ptr_a = &b;  // expected-error{{changes address space of pointer}}
  ptr_b = &a;  // expected-error{{changes address space of pointer}}

  ptr_b = ptr_a;  // expected-error{{changes address space of pointer}}
  ptr_a = ptr_b;  // expected-error{{changes address space of pointer}}

}
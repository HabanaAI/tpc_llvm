// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s 

void main(int dest) {
  float64 volatile __local *ptr = (float64 __local *)dest;

  float64 x = ptr[0];
  (*ptr++) = ~x;  // expected-error{{invalid argument type 'float64' (vector of 64 'float' values) to unary expression}}
}

// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s 

float res = 1.q; // expected-error{{float128 literal constant is not allowed}}
void main() {
  res = 11.4f; 
}


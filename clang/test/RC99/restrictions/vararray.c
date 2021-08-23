// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

void main(int array_length) {
  int int_array[array_length]; // expected-error{{variable arrays are not supported}}
}


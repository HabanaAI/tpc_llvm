// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
void main() {

  int5 y = { 0,0,0,0,0 };
  int5 x = {1, 2, 3, 4, 5 };
  int5 res;

  res = x >> y; // expected-error{{operation is not supported for int5 datatype}}
  res = x >> 5; // expected-error{{operation is not supported for int5 datatype}}
  res = 3 << y; // expected-error{{operation is not supported for int5 datatype}}
  res >>= 3;    // expected-error{{operation is not supported for int5 datatype}}
  res <<= x;    // expected-error{{operation is not supported for int5 datatype}}
}

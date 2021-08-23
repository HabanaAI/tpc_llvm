// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

__local int64 localArray[5];

void main(__global int64 *globalArray) {  // expected-error{{parameter 'globalArray' of entry function has wrong type}}
  __local int64 *pointer = &(localArray[0]);
  int64 tmp = *pointer;
  *globalArray = tmp;
}

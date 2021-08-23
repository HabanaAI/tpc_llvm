// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

short Arr0[1];

struct S0 {
  short f1;
};

struct S0 Arr1[1];

union U0 {
  short f1;
  int f2;
};

union U0 Arr2[1];

short Arr10[2];  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
struct S0 Arr11[2];  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
union U0 Arr12[2];  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}

void main() {
}

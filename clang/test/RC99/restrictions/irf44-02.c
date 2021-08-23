// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -long-irf -verify %s

void func_01(int);
int5 func_02();

void main(int x) {
  int5 ndx1, ndx2;

  ndx1[1] = ndx2[2]; // expected-error{{this access to int5 component is not allowed in long irf mode - different dimensions}}
  ndx1[3] = ndx1[3] - ndx2[2]; // expected-error{{this access to int5 component is not allowed in long irf mode - different dimensions}}

  ndx1[2] += ndx2[1]; // expected-error{{this access to int5 component is not allowed in long irf mode - different dimensions}}

  int xx = ndx1[0]; // expected-error{{this access to int5 component is not allowed in long irf mode}}
  func_01(ndx1[0]); // expected-error{{this access to int5 component is not allowed in long irf mode}}
  int yy = func_02()[0]; // expected-error{{this access to int5 component is not allowed in long irf mode}}
  _Bool res;
  res = ndx1[1] < (ndx1[2] - ndx2[2]); // expected-error 2 {{this access to int5 component is not allowed in long irf mode}}

}
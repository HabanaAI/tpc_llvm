// RUN: %clang_cc1 -fsyntax-only -std=rc++ -triple tpc-none-none -verify %s

//GAUDI-1366
// XFAIL:*

struct A {
  int x;
};

struct B : public A {
  int y;
};

bool func_01(A *x, B *y) {
  return typeid(x) == typeid(y); // expected-error{{'typeid' is not supported}}
                                 // expected-error@-1{{'typeid' is not supported}}
}

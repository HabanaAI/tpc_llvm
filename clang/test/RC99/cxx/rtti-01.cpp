// RUN: %clang_cc1 -fsyntax-only -std=rc++ -triple tpc-none-none -verify %s

struct A {
  int x;
};

struct B : public A {
  int y;
};

bool func_01(A *x) {
  if (dynamic_cast<B*>(x)) // expected-error{{'dynamic_cast' is not supported}}
    return true;
  return false;
}

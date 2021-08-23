// RUN: %clang_cc1 -fsyntax-only -std=rc++ -triple tpc-none-none -verify %s

//GAUDI-1366
// XFAIL:*

struct A {
  int x;
};

A* func_01() {
  return new A();  // expected-error{{'default new' is not supported}}
}

void main() {
}

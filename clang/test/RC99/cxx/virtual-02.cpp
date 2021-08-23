// RUN: %clang_cc1 -fsyntax-only -std=rc++ -triple tpc-none-none -verify %s

struct C1 {
  int x;
};

struct ABCD : public virtual C1 { // expected-error{{virtual base classes are not supported}}
  int y;
};

void main(int src) {
}

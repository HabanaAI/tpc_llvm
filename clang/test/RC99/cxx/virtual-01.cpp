// RUN: %clang_cc1 -fsyntax-only -std=rc++ -triple tpc-none-none -verify %s

struct ABCD {
  virtual void ff() {} // expected-error{{virtual functions are not supported}}
};

void main(int src) {
}

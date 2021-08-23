// RUN: %clang_cc1 -fsyntax-only -std=rc++ -triple tpc-none-none -verify %s

void func_01(int *x) {
  try {   // expected-error{{cannot use 'try' with exceptions disabled}}
    if (x == 0)
      throw(12);  // expected-error{{cannot use 'throw' with exceptions disabled}}
  } catch (...) {
  }
}

void main() {
}

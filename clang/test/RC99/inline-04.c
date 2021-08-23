// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -main-function entry -verify %s
int main(int x) {
  return x + 2;
}

inline void entry() {  // expected-error{{entry function is not allowed to be declared inline}}
  int y = main(12);
}

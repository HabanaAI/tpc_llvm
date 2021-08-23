// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

extern void abc();
void main() {
  abc(); // expected-error{{function is used but not defined}}
}

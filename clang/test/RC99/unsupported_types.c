// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
void call_base() {
  double   d;   // expected-error{{double is not supported on this target}}
  long double   ld;   // expected-error{{long double is not supported on this target}}
  long long   ll;   // expected-error{{long long is not supported on this target}}
}


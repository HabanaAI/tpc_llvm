// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
void main(int arg_int) {
#pragma tpc_printf enable) // expected-warning{{missing '(' after '#pragma tpc_printf' - ignoring}}
  printf_i("value is integer\n", arg_int);
}


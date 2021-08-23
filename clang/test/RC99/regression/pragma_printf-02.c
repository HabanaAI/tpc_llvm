// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
void main(int arg_int) {
#pragma tpc_printf (1) // expected-warning{{expected identifier in '#pragma tpc_printf' - ignored}}
  printf_i("value is integer\n", arg_int);
}


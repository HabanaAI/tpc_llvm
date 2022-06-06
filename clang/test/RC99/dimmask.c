// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -fsyntax-only -verify %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -fsyntax-only -verify %s

//GAUDI-1366
// XFAIL:*

void main(int mask) {
#ifdef __goya2__
// expected-no-diagnostics
  int5 res = i_i32_mov(1, mask, 0, 0, 1, 0);
#else
  int5 res = i_i32_mov(1, mask, 0, 0, 1, 0); // expected-error{{argument to 'i_i32_mov' must be a constant integer}}
#endif
}

// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

int one(int r) {
  return 1;
}

void main(int x, int y, int y2) {
  float64 __local *ptr = (float64 __local *)x;

  float res = (*ptr)[0]; // expected-error{{access to elements of type 'float64' (vector of 64 'float' values) is not allowed}}
  (*ptr)[0] = res;  // expected-error{{access to elements of type 'float64' (vector of 64 'float' values) is not allowed}}

  int64 fff = 0;
  float aaa = fff[2]; // expected-error{{access to elements of type 'int64' (vector of 64 'int' values) is not allowed}}
  fff[2] = aaa;  // expected-error{{access to elements of type 'int64' (vector of 64 'int' values) is not allowed}}

  int5 __local *ptr2 = (int5 __local *)y;
  int resi = (*ptr2)[0];
  one(fff[2]);  // expected-error{{access to elements of type 'int64' (vector of 64 'int' values) is not allowed}}

  int64 v1 = y2;
  printf_i("I", v1[2]);

  uint64 v2 = y2;
  printf_ui("UI", v2[2]);

  short128 v3 = y2;
  printf_s("S", v3[2]);

  ushort128 v4 = y2;
  printf_us("US", v4[2]);

  char256 v5 = y2;
  printf_c("C", v5[2]);

  uchar256 v6 = y2;
  printf_uc("UC", v6[2]);
}

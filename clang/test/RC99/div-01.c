// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc -verify %s
// expected-no-diagnostics

void main(int dest, int x0, int y0, float x1, float y1) {
  int __local *ptr = (int __local *)dest;
  *ptr++ = x0 / y0;
  *ptr++ /= y0;
  *ptr++ = x0 / 4;
  *ptr++ = 20 / 4;
  

  float __local *ptr2 = (float __local *)ptr;
  *ptr2++ = x1 / y1;
  *ptr2++ /= y1;
  *ptr2++ /= 2;
  *ptr++ = 20 / 4;
  *ptr++ = 1.0 / 0.0;
}

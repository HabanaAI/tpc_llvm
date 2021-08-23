// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc -verify %s

void main(int dest, int x0, int y0, float x1, float y1) {
  int __local *ptr = (int __local *)dest;
  *ptr++ = x0 / y0;  // expected-error{{division is not supported}}
  *ptr++ /= y0;      // expected-error{{division is not supported}}
  *ptr++ = x0 / 4;
  *ptr++ = 20 / 4;
  

  float __local *ptr2 = (float __local *)ptr;
  *ptr2++ = x1 / y1; // expected-error{{division is not supported}}
  *ptr2++ /= y1;     // expected-error{{division is not supported}}
  *ptr2++ /= 2;      // expected-error{{division is not supported}}
  *ptr++ = 20 / 4;
  *ptr++ = 1.0 / 0.0;
}

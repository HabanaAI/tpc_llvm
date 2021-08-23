// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -fsyntax-only -verify %s

void main(int src) {

  bool256 __local *sptr = (bool256 __local *)src;

  bool256 x = *sptr++;
  bool256 y = *sptr++;
  bool256 res;

  res = y;

  res = +x;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = -x;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res++;            // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  ++res;            // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res--;            // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  --res;            // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res = x + y;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = x + 0;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = 0 + x;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res += x;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res += 0;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res = x - y;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = 0 - y;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = x - 0;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res -= x;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res -= 0;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res = x * y;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = x * 1;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = 1 * x;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res *= x;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res *= 1;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res = x << y;     // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = x << 1;     // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = 1 << x;     // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res <<= x;        // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res <<= 1;        // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res = x >> y;     // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = x >> 1;     // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = 1 >> x;     // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res >>= x;        // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res >>= 1;        // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  (void) (x < y);   // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  (void) (x <= y);  // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  (void) (x > y);   // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  (void) (x >= y);  // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  
  res = x / y;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = x / 1;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res /= x;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res /= 1;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res = x % y;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res = x % 1;      // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}

  res %= x;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
  res %= 1;         // expected-error{{this operation is not allowed on 'bool256', 'bool128' or 'bool64' operands}}
}

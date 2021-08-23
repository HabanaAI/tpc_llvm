// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -verify %s

_BFloat16 BF16_zero_01 = 0; // expected-error {{type _Bfloat16 is not supported}}

void main() {
}


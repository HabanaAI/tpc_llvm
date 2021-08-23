// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi -verify -o - %s
// expected-no-diagnostics

_BFloat16 BF16_zero_01 = 0;

void main() {
}


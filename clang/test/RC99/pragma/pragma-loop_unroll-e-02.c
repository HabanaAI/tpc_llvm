// RUN: %clang_cc1 -fsyntax-only -verify -std=rc99 -triple tpc %s

void main(int dest, float value, int start, int end, int stride) {
  __local__ float* res=(__local float *)dest;

  #pragma clang loop machine_unroll_count  // expected-error{{expected '('}}
  for (int x = start; x < end; x += stride) {
    *res = value;
    value += 2.0;
    ++res;
  }
}

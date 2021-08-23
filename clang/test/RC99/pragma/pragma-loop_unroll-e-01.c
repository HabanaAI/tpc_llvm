// RUN: %clang_cc1 -fsyntax-only -verify -triple tpc -std=rc99 %s

void main(int dest, float value, int start, int end, int stride) {
  __local__ float* res=(__local float *)dest;

  #pragma loop_unroll	// expected-error{{missing argument; expected an integer value}}
  for (int x = start; x < end; x += stride) {
    *res = value;
    value += 2.0;
    ++res;
  }
}

// RUN: %clang_cc1 -std=rc99 -triple tpc -verify %s

void main(int dest, float value, int start, int end, int stride) {
  __local__ float* res=(__local float *)dest;

  #pragma loop_unroll(4) pipelined pipelined // expected-warning{{pragma option is already specified}}
  for (int x = start; x < end; x += stride) {
    *res = value;
    value += 2.0;
    ++res;
  }
}

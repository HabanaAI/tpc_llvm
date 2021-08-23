// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

struct New {
  float64 vec;
} var;

__local__  struct New localvar;

__global__  struct New globalvar; // expected-error{{variables in global address space are not allowed}}


struct TPC {
  __local__ int64 vec;   // expected-error{{field may not be qualified with an address space}}
};
void main() {}

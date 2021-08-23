// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
// expected-no-diagnostics

struct S1 {
  int F1;
  float F2;
  int __local *F3;
  int64 __local *F4;
  int5 F5;
};

struct S2 {
  struct S1 F1;
};

struct S3 {
  int64 F1;
  bool256 F2;
};

union U3 {
  int F1;
  float F2;
  int __local *F3;
  int64 __local *F4;
  int5 F5;
  struct S1 F6;
};

void main() {
}

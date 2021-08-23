// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

struct S1 {
  int F1;
  int64 F2;   // expected-error{{all field types must have either vector or static allocation}}
};

struct S2 {
  int64 F1;
  int F2;   // expected-error{{all field types must have either vector or static allocation}}
};

struct S3 {
  int F1;
  float F2;
  int __local *F3;
  int64 __local *F4;
  int5 F5;
};

struct S4 {
  int64 __local *F1;
  int64 F2;   // expected-error{{all field types must have either vector or static allocation}}
};

union U1 {
  int F1;
  int64 F2;   // expected-error{{all field types must have either vector or static allocation}}
};

struct U2 {
  int64 F1;
  int F2;   // expected-error{{all field types must have either vector or static allocation}}
};

struct U3 {
  int F1;
  float F2;
  int __local *F3;
  int64 __local *F4;
  int5 F5;
};

struct U4 {
  int64 __local *F1;
  int64 F2;   // expected-error{{all field types must have either vector or static allocation}}
};

void main() {
}

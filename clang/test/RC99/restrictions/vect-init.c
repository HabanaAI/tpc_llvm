// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

struct ABC {
  int64 v;
};

void main(int dest) {
  int64 f0 = 0;
  int64 f1 = 1;
  int64 f2 = dest;
  int64 f3 = { 0 };
  int64 f4 = { 1 };        // expected-error{{this vector type cannot be initialized element by element}}
                           // expected-note@-1{{omit braces to make broadcast}}
  int64 f5 = { dest };     // expected-error{{this vector type cannot be initialized element by element}}
                           // expected-note@-1{{omit braces to make broadcast}}
  int64 f6 = { 0, 0 };
  int64 f7 = { 1, 0 };     // expected-error{{this vector type cannot be initialized element by element}}
  int64 f8 = { dest, 0 };  // expected-error{{this vector type cannot be initialized element by element}}

  struct ABC s0 = { { 0 } };
  struct ABC s1 = { { 1 } };    // expected-error{{this vector type cannot be initialized element by element}}
                                // expected-note@-1{{omit braces to make broadcast}}
  struct ABC s2 = { { 0, 0 } };
  struct ABC s3 = { { 0, 1 } }; // expected-error{{this vector type cannot be initialized element by element}}
  struct ABC s4 = { { dest } }; // expected-error{{this vector type cannot be initialized element by element}}
                                // expected-note@-1{{omit braces to make broadcast}}
  struct ABC s5 = { 1 };

  int64 a0[2] = { 1 };
  int64 a1[2] = { { 0 } };
  int64 a2[2] = { { 1 } };         // expected-error{{this vector type cannot be initialized element by element}}
                                   // expected-note@-1{{omit braces to make broadcast}}
  int64 a3[2] = { dest };
  int64 a4[2] = { { dest } };      // expected-error{{this vector type cannot be initialized element by element}}
                                   // expected-note@-1{{omit braces to make broadcast}}
  int64 a5[2] = { 1, 2 };
  int64 a6[2] = { { 0 }, { 1 } };  // expected-error{{this vector type cannot be initialized element by element}}
                                   // expected-note@-1{{omit braces to make broadcast}}

  int5 idx = { 0, 0, 1 };
}

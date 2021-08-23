// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

struct S0 {
  short f1;
};

union U0 {
  short f1;
  int f2;
};

union U1 {
  union U0 f1;
  unsigned short f2;
};

union U2 {
  struct S0 f1;
  int f2;
};

union U3 {
  short f1[1];
  int f2;
};

struct S1 {
  struct S0 f1;
};

struct S2 {
  short f1[1];
};

struct S3 {
  struct S0 f1[1];
};

struct S4 {
  union U0 f1[1];
};



struct S10 {  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
  short f1;
  short f2;
};

struct S11 {  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
  struct {
    short f;
  } f1;
  int f2;
};

struct S12 {  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
  short f1[2];
};

struct S13 {  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
  struct S0 f1[2];
};

struct S14 {  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
  union U0 f1[2];
};

struct S15 {  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
  struct S0 f1;
  struct S0 f2;
};

struct S16 {  //expected-error{{arrays or structures containing more than one value shorter than 32 bits are not supported}}
  union U0 f1;
  union U0 f2;
};

void main() {
}

// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
// expected-no-diagnostics
// vector addrspace only
int Global_Var;
struct Good_Vect {
  int64 N;
  union {
    float64 F;
    short128 H;
  } U;
};

struct Users {
  bool256 User;
  struct Good_Vect Var;
  int64 NN;
};

struct Good_Vect Var;

void main()
{
  static struct Users St_Var;
  St_Var.NN = 1;
}

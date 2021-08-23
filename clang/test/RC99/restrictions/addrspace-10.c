// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
// expected-no-diagnostics
// No fields with vector addrspace
int Global_Var;
struct Good {
  int *N;
  int64 *ptr;
  union {
    int NN;
    float F;
  } U;
};

struct Users {
  int User;
  struct Good Var;
  int64 *ptr;
};

struct Good Var;

void main()
{
  static struct Users St_Var;
  St_Var.User = 1;
}

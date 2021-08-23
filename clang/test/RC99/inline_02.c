// RUN: not %clang_cc1 -S -emit-llvm -triple tpc-none-none -std=rc99 %s -o - 2>&1 | FileCheck %s

int factorial(int n);

int ret_one(int n) {
  return n*factorial(n - 1);
}

int factorial(int n)
{
  return n*ret_one(n - 1);
}


int main()
{
  return ret_one(7);
}
// CHECK: fatal error: error in backend: Function {{.*}} participates in recursion call.

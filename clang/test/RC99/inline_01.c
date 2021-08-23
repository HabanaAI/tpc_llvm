// RUN: not %clang_cc1 -S -emit-llvm -triple tpc-none-none -std=rc99 %s -o - 2>&1 | FileCheck %s

int factorial(int n)
{ 
  if (n == 1)
    return 1;
  return n*factorial(n - 1);
}


int main()
{
  return factorial(7);
}
// CHECK: fatal error: error in backend: Function 'factorial' participates in recursion call.

// RUN: %clang_cc1 -S -emit-llvm -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

int factorial(int n)
{
  return n;
}


int main()
{
  return factorial(7);
}

// CHECK-NOT: define i32 @factorial


// RUN: not %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - 2>&1 | FileCheck %s 

int gval[257];

void main(int x) {
  *(int __local *)x = gval[0];
}

// CHECK: too much scalar memory is used for statically allocated data: 1028 is allocated, but only 1024 is available
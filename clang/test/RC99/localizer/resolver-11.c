// RUN: not %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - 2>&1 | FileCheck %s 

int64 gval[321];

void main(int x) {
  *(int64 __local *)x = gval[0];
}

// CHECK: too much vector memory is used for statically allocated data: 82176 is allocated, but only 81920 is available
// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

void main(_Bool x) {
  *(int __local *)0x100 = x;
}

// CHECK: st_l 0x100, %S0

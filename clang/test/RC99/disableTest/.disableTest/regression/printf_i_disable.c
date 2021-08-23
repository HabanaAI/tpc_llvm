// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s

void main(tensor t0, tensor t1, int arg_int) {

#pragma tpc_printf (disable)
  printf_i("value is integer\n", arg_int);
}

// CHECK:   define void @main(
// CHECK:     ret void
// CHECK-NEXT: }

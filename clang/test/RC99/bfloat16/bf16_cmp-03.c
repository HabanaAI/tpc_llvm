// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, _BFloat16 src1, _BFloat16 src2) {
  _Bool __local *dptr = (_Bool __local *) dest;

  *dptr = src1 < src2;
// CHECK: cmp_less.bf16 %SP{{[0-9]+}}, %S1, %S2
}

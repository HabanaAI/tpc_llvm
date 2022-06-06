// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi2 -float8 %s -o - | FileCheck %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu doron1 -float8 %s -o - | FileCheck %s

void main(int dest, int src) {
  _Float8_143 __local *dptr = (_Float8_143 __local *) dest;
  _Float8_143 __local *sptr = (_Float8_143 __local *) src;

  *dptr = *sptr;
// CHECK: ld_l  [[DST1:%S[0-9]+]], [[REG:%S[0-9]+]]
// CHECK: st_l  [[DST2:%S[0-9]+]], [[DST1]]
}

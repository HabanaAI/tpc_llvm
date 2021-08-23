// RUN: %clang_cc1 -triple -tpc-none-none -std=rc99 -S -O1 -target-cpu dali %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple -tpc-none-none -std=rc99 -S -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(int dest) {
  char256 __local *dptr = (char256 __local *) dest;
  *dptr = read_lfsrnc() + read_lfsrnc();
// CHECK:      READ    %V{{[0-9]+}}, LFSR_NO_CHANGE
// CHECK-NOT:  LFSR_NO_CHANGE
}

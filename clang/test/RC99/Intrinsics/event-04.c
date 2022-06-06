// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(unsigned int x1, unsigned int x2, _Bool p) {
  event(x1, 0, p, 0);
// CHECK-DAG: event spu %S[[NDX2:[0-9]+]], %SP[[NDX2:[0-9]+]]
  event(x2, 1, p, 0);
// CHECK-DAG: event vpu %S[[NDX2:[0-9]+]], %SP[[NDX2:[0-9]+]]
  event(123, 0, p, 0);
// CHECK-DAG: event spu 0x7b, %SP[[NDX2:[0-9]+]]
  event(321, 1, p, 0);
// CHECK-DAG: event vpu 0x141, %SP[[NDX2:[0-9]+]]
}

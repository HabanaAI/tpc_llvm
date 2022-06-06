// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// TODO: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(tensor in, int value2, _Bool pred) {
  int5 coords = 0; 
  coords = set_indx(123, coords, 1, 0, pred, 0);
  int __global *ptr = (int __global *) gen_addr(coords, in, 0, 0, 1, 0);
  *ptr = value2;
}

// CHECK-DAG:   mov      %SP[[PRED:[0-9]+]], %S1
// CHECK-DAG:   set_indx %I[[NDX:[0-9]+]], b11111, 0x0
// CHECK:       set_indx %I[[NDX]], b00001, 0x7b, %SP[[PRED]]
// CHECK-NEXT:  gen_addr %AD[[ADDR:[0-9]+]], 0x0, %I[[NDX]]
// CHECK:       st_g     %AD[[ADDR]], %S0

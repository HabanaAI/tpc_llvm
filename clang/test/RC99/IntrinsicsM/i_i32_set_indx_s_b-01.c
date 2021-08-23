// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(tensor in, int value, int value2, _Bool pred) {
  int5 coords = 0; 
  coords = i_i32_set_indx_s_b(value, coords, 1, pred, 0);
  int __global *ptr = (int __global *) a_gen_addr_i(coords, in);
  *ptr = value2;
}

// CHECK-DAG:   mov      %SP[[PRED:[0-9]+]], %S2
// CHECK-DAG:   set_indx %I[[NDX:[0-9]+]], b11111, 0x0, %SP0
// CHECK:       set_indx %I[[NDX]], b00001, %S0, %SP[[PRED]]
// CHECK-NEXT:  gen_addr %AD[[ADDR:[0-9]+]], 0x0, %I[[NDX]], %SP0
// CHECK:       st_g     %AD[[ADDR]], %S1, %SP0

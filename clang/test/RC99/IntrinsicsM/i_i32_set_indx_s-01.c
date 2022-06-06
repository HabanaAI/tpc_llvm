// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// TODO: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(tensor in, int value, int value2) {
  int5 coords = 0; 
  coords = set_indx(value, coords, 1, 0, 1, 0);
  int __global *ptr = (int __global *) gen_addr(coords, in, 0, 0, 1, 0);
  *ptr = value2;
}

// CHECK:       set_indx %I[[NDX:[0-9]+]], b11111, 0x0
// CHECK:      set_indx %I[[NDX]], b00001, %S0
// CHECK:       gen_addr %AD[[ADDR:[0-9]+]], 0x0, %I[[NDX]]
// CHECK:       st_g     %AD[[ADDR]], %S1

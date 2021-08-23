// RUN: %codegen -triple -tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

void main(int dest, int dest2, int src) {
  uint64 __local *ptr = (uint64 __local*)dest;

  *ptr = (uint64) LFSR;
// CHECK: READ %V{{[0-9]+}}, LFSR

  ptr[1] = (uint64) LFSR_NO_CHANGE;
// CHECK-DAG: READ %V{{[0-9]+}}, LFSR_NO_CHANGE

  ptr[2] = V_LANE_ID_32;
// CHECK-DAG: st_l_v %S0, 0x200, %V_LANE_ID_32

  uchar256 __local *ptr2 = (uchar256 __local*)dest2;

  *ptr2 = V_LANE_ID_8;
// CHECK: st_l_v %S1, 0x0, %V_LANE_ID_8
}

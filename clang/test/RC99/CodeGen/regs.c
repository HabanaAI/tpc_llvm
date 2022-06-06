// RUN: %codegen -triple -tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

void main(int dest, int dest2, int src) {
  uint64 __local *ptr = (uint64 __local*)dest;

  *ptr = (uint64) read_lfsr_b();
  //CHECK-DAG: READ %V{{[0-9]+}}, LFSR

  ptr[1] = (uint64) read_lfsr_b(SW_READ_ONLY);
  //CHECK-DAG: READ %V{{[0-9]+}}, LFSR_NO_CHANGE

  ptr[2] = read_lane_id_4b_b();
  //CHECK-DAG: READ %V{{[0-9]+}}, V_LANE_ID_32

  uchar256 __local *ptr2 = (uchar256 __local*)dest2;

  *ptr2 = read_lane_id_1b_b();
  //CHECK-DAG: READ %V{{[0-9]+}}, V_LANE_ID_8
}

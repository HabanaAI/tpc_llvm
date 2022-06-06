// RUN: %tpc_clang -O1 -c -x c++ -march=goya2 %s -o %t.o && %metadata_disasm --mcpu goya2 %t.o | FileCheck %s

void main(int src, int dest) {
  int5 ndx = { 1, 0, 1, 0, 0 };
  float64 val = src;
  v_f32_st_tnsr_rmw(ndx, dest, val, MkRMW(e_rmw_fp32, e_rmw_add, e_rmw_atomic, e_tnsr_dt_srf), 0, 1, 0);
}

// CHECK: rmwStore[0]: DB 1
// CHECK: rmwStore[1]: DB 1
// CHECK: rmwStore[2]: DB 1
// CHECK: rmwStore[3]: DB 1
// CHECK: rmwStore[4]: DB 1
// CHECK: rmwStore[5]: DB 1
// CHECK: rmwStore[6]: DB 1
// CHECK: rmwStore[7]: DB 1
// CHECK: rmwStore[8]: DB 1
// CHECK: rmwStore[9]: DB 1
// CHECK: rmwStore[10]: DB 1
// CHECK: rmwStore[11]: DB 1
// CHECK: rmwStore[12]: DB 1
// CHECK: rmwStore[13]: DB 1
// CHECK: rmwStore[14]: DB 1
// CHECK: rmwStore[15]: DB 1
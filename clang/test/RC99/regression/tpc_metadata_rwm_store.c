// RUN: %tpc_clang -O1 -c -x c++ -march=goya2 %s -o %t.o && %metadata_disasm --mcpu goya2 %t.o | FileCheck %s

void main(int src, int dest) {
  int5 ndx = { 1, 0, 1, 0, 0 };
  int64 val = src;
  v_i32_st_tnsr_rmw(ndx, 3, val, 0, 0, 0, 1);
}

// CHECK: rmwStore[3]: DB 1

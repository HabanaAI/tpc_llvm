// RUN: %tpc_clang -O1 -c -x c -march=goya2 %s -o %t.o && %metadata_disasm --mcpu goya2 %t.o | FileCheck %s

void main(int addr, float value, _Bool pred) {
  s_f32_st_l(addr, value, 0, pred, 0);
}

// CHECK: mmioUsed: DB 0

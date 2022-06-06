// RUN: %tpc_clang -O1 -c -x c -march=goya2 %s -o %t.o && %metadata_disasm --mcpu goya2 %t.o | FileCheck %s

void main(int addr, int dest, _Bool pred) {
  float __local *ptr_dest = (float __local *)dest;
  float res = s_f32_ld_l(addr, 0, res, pred, 0);
  *ptr_dest++ = res;
}

// CHECK: mmioUsed: DB 0

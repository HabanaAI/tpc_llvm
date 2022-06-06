// RUN: %tpc_clang -c --target=tpc -march=dali  -mllvm -tpc-asm-format=3  %s -S -o - 2>&1 | FileCheck %s

void main(tensor tensor0, int src,int dst1,int dst2) {
  int5 offset = {0,0,0,0,0};

  int storeCoord[5] = { 0, 1, 2, 3, 4 };
  int a = storeCoord[src];
  __global__ void* addr = gen_addr(offset, tensor0, 0, 0, 1, 0);
  s_i32_st_g(addr, a, 0, 1, 0);

}
//CHECK:     ld_l  S{{[0-9]+}}, S{{[0-9]+}};   nop;    nop;    nop

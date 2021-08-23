// RUN: %tpc_clang -c --target=tpc -march=dali  -mllvm -tpc-asm-format=3  %s -S -o - 2>&1 | FileCheck %s

void main(tensor tensor0, int src,int dst1,int dst2) {
  int5 offset = {0,0,0,0,0};

  int storeCoord[5] = { 0, 1, 2, 3, 4 };
  int a = storeCoord[src];
  __global__ void* addr = a_gen_addr_i(offset, tensor0);
  i32_st_g_a_s_b(addr, a,1,0);

}
//CHECK:     ld_l  S{{[0-9]+}}, S{{[0-9]+}}, SP0;   nop;    nop;    nop

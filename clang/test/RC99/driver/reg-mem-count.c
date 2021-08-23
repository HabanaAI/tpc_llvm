// RUN: %tpc_clang -reg-mem-count -mllvm -tpc-addr-opt=0  %s -S -o - 2>&1 | FileCheck %s

void main(tensor tensor0, int src,int dst1,int dst2) {
  int5 offset = {0,0,0,0,0};

  int storeCoord[5] = { 0, 1, 2, 3, 4 };
  int a = storeCoord[src];
  __global__ void* addr = a_gen_addr_i(offset, tensor0);
  i32_st_g_a_s_b(addr, a,1,0);

  int64 val[5] = {src,src,src,src,src};
  int64 b = val[src];
  bool256 pred= bv_i32_cmp_eq_v_v(b,b);
  int64 res = 0;
  res = v_i32_add_v_v_vb(b,val[1],res,1,pred,0);
  i32_st_tnsr_i_v(offset,tensor0,res);
}

//CHECK: Total SLM used: 20 bytes
//CHECK: Total VLM used: 1280 bytes
//CHECK: 2 VRF registers used: %V0 %V1
//CHECK: {{[0-9]+}} SRF registers used: %S0 %S1 %S2 %S3 %S4 %S5 %S6 %S7 %S8
//CHECK: 1 VPRF register used: %VP1
//CHECK: 1 SPRF register used: %SP0
//CHECK: 1 IRF register used: %I2
//CHECK: 1 ADRF register used: %AD0


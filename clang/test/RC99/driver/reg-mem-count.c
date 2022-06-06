// RUN: %tpc_clang -reg-mem-count  %s -S -o - 2>&1 | FileCheck %s

void main(tensor tensor0, int src,int dst1,int dst2) {
  int5 offset = {0,0,0,0,0};

  int storeCoord[5] = { 0, 1, 2, 3, 4 };
  int a = storeCoord[src];
  __global__ void* addr = gen_addr(offset, tensor0, 0, 0, 1, 0);
  s_i32_st_g(addr, a, 0, 1, 0);

  int64 val[5] = {src,src,src,src,src};
  int64 b = val[src];
  bool256 pred= from_bool64(v_i32_cmp_eq_b(b, b, 0, to_bool64((bool256){0}), 1, 0));
  int64 res = 0;
  res = v_i32_add_vb(b, val[1], 1, res, to_bool64(pred), 0);
  v_i32_st_tnsr(offset, tensor0, res, 0, 1, 0);
}

//CHECK: Total SLM used: 20 bytes
//CHECK: Total VLM used: 1280 bytes
//CHECK: 2 VRF registers used: %V0 %V1
//CHECK: {{[0-9]+}} SRF registers used: %S0 %S1 %S2 %S3 %S4 %S5 %S6 %S7 %S8
//CHECK: 1 VPRF register used: %VP1
//CHECK: 1 SPRF register used: %SPRF_TRUE
//CHECK: 1 IRF register used: %I2
//CHECK: 1 ADRF register used: %AD0


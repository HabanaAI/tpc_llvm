// ld_g and st_g that step their address write the address register they read.
// The stepped address was a free output, so the register allocator could put
// it elsewhere and the next access read a stale address.
//
// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s
// RUN: %tpc_clang -march=gaudi3 -O2 -S %s -o - | FileCheck %s
// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -stop-after=twoaddressinstruction %s -o - | FileCheck --check-prefix=MIR %s

void main(tensor in, tensor masks, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  int __global *q = (int __global *)gen_addr(c, out, 0, 0, 1, 0);
  s_i32_st_g_inc(&q, 101, SW_INC_8, 1, 0);
  s_i32_st_g_inc(&q, 102, SW_INC_4, 1, 0);
  *q = 103;

  int5 r = {0, 0, 0, 0, 0};
  void __global *a = gen_addr(r, in, 0, 0, 1, 0);
  int64 v1 = 0, v2 = 0;
  v1 = v_i32_ld_g_inc_partial_b(&a, 15, 0, 0, v1, 1, 0);
  v2 = v_i32_ld_g_inc_partial_b(&a, 15, 0, 0, v2, 1, 0);

  r[1] = 1;
  void __global *b = gen_addr(r, in, 0, 0, 1, 0);
  int5 x = 0, y = 0, z = 0;
  x = i_i32_ld_g_inc(&b, 0b00011, 0, x, 1, 0);
  y = i_i32_ld_g_inc(&b, 0b00011, 0, y, 1, 0);
  int5 m = {0, 0, 0, 0, 0};
  int mask = s_i32_ld_g((int __global *)gen_addr(m, masks, 0, 0, 1, 0), 0, 0, 1, 0);
  z = i_i32_ld_g_inc(&b, mask, 0, z, 1, 0);

  int5 o = {0, 2, 0, 0, 0};
  v_i32_st_tnsr(o, out, v1);
  o[1] = 3; v_i32_st_tnsr(o, out, v2);
  o[1] = 4; v_i32_st_tnsr(o, out, x[0] + y[1] + z[4]);
}

// CHECK-DAG: st_g inc_8 [[Q:%AD[0-9]+]], %S
// CHECK-DAG: st_g inc_4 [[Q]], %S
// CHECK-DAG: st_g [[Q]], %S
// CHECK-DAG: ld_g inc_4 %V{{[0-9]+}}, [[A:%AD[0-9]+]], %LD_PARTIAL_REG
// CHECK-DAG: ld_g inc_4 %V{{[0-9]+}}, [[A]], %LD_PARTIAL_REG
// CHECK-DAG: ld_g inc_4 b00011 %I{{[0-9]+}}, [[B:%AD[0-9]+]]
// CHECK-DAG: ld_g inc_4 b00011 %I{{[0-9]+}}, [[B]]
// CHECK-DAG: ld_g inc_4 %M0 %I{{[0-9]+}}, [[B]]

// After two-address lowering each stepped address is the register it reads.
// MIR: [[R1:%[0-9]+]]:adrf = ST_G_INCs [[R1]],
// MIR: [[R2:%[0-9]+]]:adrf = ST_G_INCs [[R2]],
// MIR: %{{[0-9]+}}:vrf, [[R3:%[0-9]+]]:adrf = LD_G_P_INCg4vap [[R3]],
// MIR: %{{[0-9]+}}:vrf, {{(dead )?}}[[R4:%[0-9]+]]:adrf = LD_G_P_INCg4vap [[R4]],
// MIR: %{{[0-9]+}}:irf, [[R5:%[0-9]+]]:adrf = LD_G_INCg4Iap [[R5]],
// MIR: %{{[0-9]+}}:irf, [[R6:%[0-9]+]]:adrf = LD_G_INCg4Iap [[R6]],
// MIR: %{{[0-9]+}}:irf, {{(dead )?}}[[R7:%[0-9]+]]:adrf = LD_G_INCg4Iwap [[R7]],

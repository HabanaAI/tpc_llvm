// RUN: %tpc_clang -S -O2 -march=gaudi2 %s -o - | FileCheck %s --check-prefixes=CHECK,X2
// RUN: %tpc_clang -S -O2 -march=gaudi3 %s -o - | FileCheck %s --check-prefixes=CHECK,X2
// RUN: %tpc_clang -S -O2 -march=goya2 %s -o - | FileCheck %s --check-prefixes=CHECK,NOX2
// RUN: %tpc_clang -S -O2 -march=gaudi %s -o - | FileCheck %s --check-prefix=GAUDI
// RUN: %tpc_clang -S -O1 -march=gaudi2 %s -o - | FileCheck %s --check-prefix=O1

// TPCMulAddCombine. A product in f32 is v_bf16_mac_acc32_b(v, w, 0): a MAC
// into a zeroed register pair, which had to be zeroed again (two MOVs) before
// every MAC. As a MADD with a +0 addend it reads one zero, made once before
// the loop. MAC and MADD give the same values here: a*b + 0, rounded once.
// The two f32 MULs of the halves of the product ('p.v1 *= r; p.v2 *= r;')
// become one MUL x2 on Gen4+, where it exists.
//
// Gaudi has no MADD; -O1 does not run the pass.

void main(tensor y, tensor g, tensor out, int n) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;
  c[1] = 1;
  float64 r = v_f32_ld_tnsr_b(c, g);
  for (int x = start[0] * 128; x < end[0] * 128; x += 128) {
    c[0] = x;
    bfloat128 v = v_bf16_ld_tnsr_b(c, y);
    bfloat128 w = v_bf16_ld_tnsr_b(c, g);
    float128 p = v_bf16_mac_acc32_b(v, w, (float128){0});
    p.v1 *= r;
    p.v2 *= r;
    v_bf16_st_tnsr(c, out, convert_float128_to_bfloat128(p, 0));
  }
}

// CHECK:      mov.f32 [[Z:%V[0-9]+]], 0x0
// CHECK:      loop
// CHECK-NOT:  mov.f32 {{.*}}, 0x0
// CHECK:      madd.bf16 acc_fp32 [[P:%D[0-9]+]], %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}
// CHECK-NOT:  mac.bf16
// X2:         mul.f32 x2 [[P]], [[P]], [[R:%V[0-9]+]], [[R]]
// NOX2-COUNT-2: mul.f32 %V{{[0-9]+}},
// CHECK:      convert.f32 all_lanes target_type=bf16 rhne %V{{[0-9]+}}, [[P]]

// GAUDI:      loop
// GAUDI:      mac.bf16 acc_fp32
// GAUDI-NOT:  madd
// GAUDI-COUNT-2: mul.f32 %V{{[0-9]+}},

// O1:         mac.bf16 acc_fp32
// O1-NOT:     madd
// O1-NOT:     mul.f32 x2

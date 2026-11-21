// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s

// MAC x2 reads the multiplier of its high half (the third input) from
// SRC_D, as MUL x2 does. The latency query took it for the accumulator in
// SRC_C, so a MUL feeding it got 4 cycles instead of 6 and the MAC read the
// old value.

void main(tensor a, tensor b, tensor c, tensor d, tensor o1, tensor o2) {
  int5 i = 0;
  float64 x = v_f32_ld_tnsr_b(i, a);
  float64 z = v_f32_ld_tnsr_b(i, b);
  float128 acc = {v_f32_ld_tnsr_b(i, c), v_f32_ld_tnsr_b(i, d)};
  float128 val = {x, z};
  float64 k = v_f32_ld_tnsr_b(i, d);
  float64 rc = k * 2.5f;
  float128 r = v_f32_mac_x2_b(val, x, rc, acc, 0, 1, 0);
  v_f32_st_tnsr(i, o1, r.v1);
  v_f32_st_tnsr(i, o2, r.v2);
}

// CHECK:      mul.f32 %V[[RC:[0-9]+]], %V{{[0-9]+}}, 0x40200000
// CHECK-NEXT: {{NOP|nop}}
// CHECK-NEXT: {{NOP|nop}}
// CHECK-NEXT: {{NOP|nop}}
// CHECK-NEXT: {{NOP|nop}}
// CHECK-NEXT: {{NOP|nop}}
// CHECK-NEXT: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %V[[RC]]

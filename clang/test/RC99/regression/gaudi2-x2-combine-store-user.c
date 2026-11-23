// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - -mllvm -enable-tpc-local-memory-forwarding=false | FileCheck %s

// The MAC x2 combine walks the users of a MAC between it and its partner. A
// user need not define a register: the first operand of a local memory store
// with an immediate address is the address, and reading it as a register
// asserted.

void main(tensor a, tensor b, tensor out) {
  int5 c = 0;
  float64 acc0 = 0.0f, acc1 = 0.0f;
  for (int i = 0; i < 4; i++) {
    c[0] = i * 64;
    float64 x = v_f32_ld_tnsr_b(c, a), y = v_f32_ld_tnsr_b(c, b);
    acc0 = v_f32_mac_b(x, y, acc0);
    v_f32_st_l_v(i * 256, acc0, 0, 1, 0);
    acc1 = v_f32_mac_b(y, y, acc1);
  }
  for (int i = 0; i < 4; i++) {
    c[0] = i * 64;
    v_f32_st_tnsr(c, out, acc1 + v_f32_ld_l_v_b(i * 256, 0, (float64)0, 1, 0));
  }
}

// CHECK: st_l_v 0x100,
// CHECK: halt

// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -verify-machineinstrs -mllvm -stop-after=early-machinelicm %s -o - 2>&1 | FileCheck %s

// The MAC x2 combine pairs two MACs whose first operands are the halves of
// one register pair. Its new instructions set no kill flags on the operands it
// takes from the two MACs (they move to another place) and kill the result
// only at its last read: the first COPY of its two halves was marked as the
// last use, and the machine verifier stopped with "Using a killed virtual
// register" after the combiner.

void main(tensor x, tensor w, tensor out) {
  int5 i = 0;
  float64 a0 = 0, a1 = 0, b0 = 0, b1 = 0;
  for (int k = 0; k < 1024; k += 128) {
    i[0] = k;
    const float128 v = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(i, x), SW_LINEAR);
    const float64 w0 = v_f32_ld_tnsr_b(i, w);
    i[0] = k + 64;
    const float64 w1 = v_f32_ld_tnsr_b(i, w);
    a0 = v_f32_mac_b(v.v1, w0, a0);
    a1 = v_f32_mac_b(v.v2, w1, a1);
    b0 = v_f32_mac_b(v.v1, w1, b0);
    b1 = v_f32_mac_b(v.v2, w0, b1);
  }
  i[0] = 0;
  v_f32_st_tnsr(i, out, a0 + b0);
  i[0] = 64;
  v_f32_st_tnsr(i, out, a1 + b1);
}

// CHECK-NOT: Bad machine code
// CHECK:     MACx2f32vvvp
// CHECK-NOT: Bad machine code

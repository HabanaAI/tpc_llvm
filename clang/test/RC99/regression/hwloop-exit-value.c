// RUN: %tpc_clang -S -O2 -march=gaudi2 -mllvm -tpc-chain-unroll-count=0 %s -o - | FileCheck %s
// RUN: %tpc_clang -S -O2 -march=gaudi -mllvm -tpc-chain-unroll-count=0 %s -o - | FileCheck %s
// RUN: %tpc_clang -S -O2 -march=gaudi3 -mllvm -tpc-chain-unroll-count=0 %s -o - | FileCheck %s

// A hardware loop that does not run goes to its exit from the LOOP's block,
// so the value of acc after the loop is a PHI of its initial value there and
// of the last MAC. That initial value was a second zero pair, not the one the
// loop starts from, and the register allocator kept the accumulator and the
// value after the loop in two pairs: the loop ended with a copy of the pair
// (two MOVs and the NOPs before them) in every iteration. The exit now takes
// the loop's own initial register and the MAC accumulates in place.

void main(tensor x, tensor out) {
  const int n = get_dim_size(x, 0);
  int5 c = 0;
  float128 acc = {0};
  for (int i = 0; i < n; i += 128) {
    c[0] = i;
    bfloat128 v = v_bf16_ld_tnsr_b(c, x);
    acc = v_bf16_mac_acc32_b(v, v, acc);
  }
  c[0] = 0;
  v_f32_st_tnsr(c, out, acc.v1 + acc.v2);
}

// CHECK:      loop 0, %S{{[0-9]+}}, 128, <, [[EXIT:\.LBB0_[0-9]+]]
// CHECK:      mac.bf16 acc_fp32 %D[[#ACC:]], [[V:%V[0-9]+]], [[V]]
// CHECK-NOT:  mov
// CHECK:      [[EXIT]]:
// CHECK:      add.f32 %V{{[0-9]+}}, %V[[#ACC]], %V[[#ACC+1]]

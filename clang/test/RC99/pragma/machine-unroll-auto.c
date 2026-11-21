// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 %s -o - 2>&1 | FileCheck --check-prefix=AUTO %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O3 -target-cpu gaudi %s -o - 2>&1 | FileCheck --check-prefix=AUTO %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -mllvm -tpc-machine-unroll-count=0 %s -o - 2>&1 | FileCheck --check-prefix=ONCE %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DINDEX %s -o - 2>&1 | FileCheck --check-prefix=AUTO %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DINDEX -mllvm -tpc-machine-unroll-index-space=false %s -o - 2>&1 | FileCheck --check-prefix=ONCE %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DACC %s -o - 2>&1 | FileCheck --check-prefix=ACC %s

// Without a machine_unroll pragma, the backend unrolls a single-block
// hardware loop 2 or 4 times when that is safe and pays (TPCPipeliner::
// autoUnrollCount): the loop counts up with '<'; its carried values are
// linear induction variables; no load or store can depend on another; the
// registers fit; and the latency estimate saves at least 3 cycles per
// iteration. That includes a loop over the member's part of the index space
// (-DINDEX), which is long unless the index space is small. An epilogue loop
// runs the iterations left over.
//
// Here (the second pass of RMSNorm) an iteration is a chain of ld, mac (madd
// where TPCMulAddCombine makes one), mul, convert and st with NOPs between
// them; four copies overlap.

void main(tensor y, tensor g, tensor out, float r) {
  const int n = get_dim_size(y, 0);
  int5 c = 0;
#if defined(ACC)
  float128 acc = {0};
  for (int x = 0; x < n; x += 128) {
    c[0] = x;
    bfloat128 v = v_bf16_ld_tnsr_b(c, y);
    acc = v_bf16_mac_acc32_b(v, v, acc);
  }
  c[0] = 0;
  v_f32_st_tnsr(c, out, acc.v1 + acc.v2);
#else
#if defined(INDEX)
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  for (int x = start[0] * 128; x < end[0] * 128; x += 128) {
#else
  for (int x = 0; x < n; x += 128) {
#endif
    c[0] = x;
    bfloat128 v = v_bf16_ld_tnsr_b(c, y);
    bfloat128 w = v_bf16_ld_tnsr_b(c, g);
    float128 p = v_bf16_mac_acc32_b(v, w, (float128){0});
    p.v1 *= r;
    p.v2 *= r;
    v_bf16_st_tnsr(c, out, convert_float128_to_bfloat128(p, 0));
  }
#endif
}

// AUTO-NOT:     warning
// AUTO:         loop {{.*}}, 512, <, [[REST:\.LBB0_[0-9]+]]
// AUTO-COUNT-4: {{(mac|madd)}}.bf16 acc_fp32
// AUTO-NOT:     {{(mac|madd)}}.bf16
// AUTO:         [[REST]]:
// AUTO-NOT:     {{(mac|madd)}}.bf16
// AUTO:         loop {{.*}}, 128, <,
// AUTO:         {{(mac|madd)}}.bf16 acc_fp32
// AUTO-NOT:     {{(mac|madd)}}.bf16
// AUTO:         halt

// ONCE:         loop {{.*}}, 128, <,
// ONCE:         {{(mac|madd)}}.bf16 acc_fp32
// ONCE-NOT:     {{(mac|madd)}}.bf16
// ONCE-NOT:     loop
// ONCE:         halt

// An accumulator is not a linear induction variable: one loop, one MAC.
// ACC-NOT:      warning
// ACC:          loop {{.*}}, 128, <,
// ACC:          mac.bf16 acc_fp32
// ACC-NOT:      mac.bf16
// ACC-NOT:      loop
// ACC:          halt

// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-modulo-schedule=false %s -o - | FileCheck %s

// A select between two 128-lane f32 values (register pairs) under a scalar
// condition is done half by half, each half from the same half of the chosen
// value. The lowering took both halves from the halves of one value (low =
// a.lo or a.hi, high = b.lo or b.hi), and 1cat's control_rrms_bf16_weight got
// wrong sums where the unbranch pass selected between two accumulators.

void main(tensor x, tensor w, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 i = 0;
  for (int y = start[1]; y < end[1]; y++) {
    i[1] = y;
    const bfloat128 a = v_bf16_ld_tnsr_b(i, x);
    const bfloat128 b = v_bf16_ld_tnsr_b(i, w);
    float128 acc = {0};
    for (int k = 0; k < 4; k++) {
      if (y & 1)
        acc = v_bf16_mac_acc32_b(a, b, acc, 0);
      else
        acc = v_bf16_mac_acc32_b(a, a, acc, 0);
    }
    i[0] = 0;
    v_f32_st_tnsr(i, out, acc.v1);
    i[0] = 64;
    v_f32_st_tnsr(i, out, acc.v2);
    i[0] = 0;
  }
}

// CHECK:     mov %V[[#LO:]], %V[[#SRC:]], !%SP
// CHECK:     mov %V[[#LO+1]], %V[[#SRC+1]], !%SP

// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - 2>&1 | FileCheck %s
// RUN: not %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-pragma-chain-unroll-addresses=0 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR

// Unrolled by 6 in order, each copy computing 2 global addresses that LICM
// keeps live across the block loop, the loop needs 12 address registers of
// the 8. An address register cannot be spilled: it also holds its tensor, so
// a load through one reloaded from a scalar pair fails (gaudi2 simulator:
// "Wrong tensorDescriptorElementSizeType"; 1cat's w2_reduce_n256 and three
// others). The unroll count is lowered to 4 (8 addresses), and a spill of an
// address register is an error rather than code that fails.

void main(tensor ids, tensor scale, tensor channel, tensor out) {
  const int5 start = get_index_space_offset(), end = start + get_index_space_size();
  for (int token = start[1]; token < end[1]; ++token) {
    for (int block = start[0]; block < end[0]; ++block) {
      float64 acc = 0;
#pragma loop_unroll(6)
      for (int slot = 0; slot < 6; ++slot) {
        const int row = token * 6 + slot;
        const int e = s_i32_ld_g(gen_addr((int5){row}, ids));
        const float sx = s_f32_ld_g(gen_addr((int5){0, row}, scale));
        acc += v_f32_ld_tnsr_b((int5){block * 64, e}, channel) * sx;
      }
      v_f32_st_tnsr((int5){block * 64, token}, out, acc);
    }
  }
}

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK:     ld_g

// ERR: too many addresses are live at once: an address register (a gen_addr result) cannot be spilled

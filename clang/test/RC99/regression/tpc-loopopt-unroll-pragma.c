// RUN: %tpc_clang -march=gaudi2 -O2 -c %s -o /dev/null

// With an IR unroll pragma on an inner loop of run-time trip count, the
// parent loop also holds the remainder loop. TpcLoopOpt took the parent's
// second block for the unrolled loop's block, found none of the loads there
// and inserted after a copy it had not placed: a crash in
// TpcLoopOptImpl::prologue. Habana's compiler crashes on it too.


void main(tensor a, tensor out, int n) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    #pragma unroll(4)
    for (int y = start[1] * n; y < end[1] * n; y++) {
      c[1] = y;
      v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, a) * 0.5f + 1.0f);
    }
  }
}

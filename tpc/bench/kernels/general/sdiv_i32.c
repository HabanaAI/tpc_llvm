// Signed vector division, i32. TPC has no vector divide instruction, so
// each / expands to a long ALU sequence: ALU-bound.
//
// BENCH: arch gaudi2
// BENCH: in  i32 2048 128 range=-2000000000,2000000000
// BENCH: in  i32 2048 128 range=-100000,100000
// BENCH: out i32 2048 128
// BENCH: index-space 32 4

#define ROWS 32

void main(tensor a, tensor b, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      int64 n = v_i32_ld_tnsr_b(c, a);
      int64 d = v_i32_ld_tnsr_b(c, b) | 1; // never 0
      v_i32_st_tnsr(c, out, n / d);
    }
  }
}

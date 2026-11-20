// u8 saturating add with 256 lanes per vector, four times the elements of an
// f32 vector per instruction.
//
// BENCH: arch gaudi2
// BENCH: in  u8 16384 128
// BENCH: in  u8 16384 128
// BENCH: out u8 16384 128
// BENCH: index-space 64 4

#define ROWS 32

void main(tensor a, tensor b, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 256; x < end[0] * 256; x += 256) {
    c[0] = x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      v_u8_st_tnsr(c, out, v_u8_add_b(v_u8_ld_tnsr_b(c, a), v_u8_ld_tnsr_b(c, b), SW_SAT));
    }
  }
}

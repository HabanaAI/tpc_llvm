// MAC x2 (gaudi2): acc.v1 += a.v1 * b, acc.v2 += a.v2 * c, with b and c the
// results of MULs just before it, so the MAC must wait for both.
//
// DIFFTEST: arch gaudi2 gaudi3
// DIFFTEST: in  f32 64 16
// DIFFTEST: in  f32 64 16
// DIFFTEST: in  f32 64 16
// DIFFTEST: in  f32 64 16
// DIFFTEST: out f32 64 16
// DIFFTEST: out f32 64 16
// DIFFTEST: index-space 1 16

void main(tensor a, tensor b, tensor c, tensor d, tensor o1, tensor o2) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 i = 0;
  for (int y = start[1]; y < end[1]; y++) {
    i[1] = y;
    float64 x = v_f32_ld_tnsr_b(i, a);
    float64 z = v_f32_ld_tnsr_b(i, b);
    float128 acc = {v_f32_ld_tnsr_b(i, c), v_f32_ld_tnsr_b(i, d)};
    float128 val = {x, z};
    float128 r = v_f32_mac_x2_b(val, x * 1.5f, z * 2.5f, acc, 0, 1, 0);
    v_f32_st_tnsr(i, o1, r.v1);
    v_f32_st_tnsr(i, o2, r.v2);
  }
}

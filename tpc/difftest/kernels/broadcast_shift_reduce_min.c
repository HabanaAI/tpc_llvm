// v_broadcast_element_f32 and v_element_shift_up_f32 for every element or
// shift from -200 to 199 and at INT_MIN and INT_MAX (outside 0..63 the result
// follows Habana's instruction sequence), and v_i32_reduce_min.
//
// DIFFTEST: arch gaudi2
// DIFFTEST: in  f32 64 64
// DIFFTEST: out f32 64 404
// DIFFTEST: out f32 64 404
// DIFFTEST: out i32 64 64
// DIFFTEST: index-space 1
// DIFFTEST: scalar i32 -200
// DIFFTEST: scalar i32 2147483647
// DIFFTEST: scalar i32 -2147483648

void main(tensor in, tensor bcast, tensor shift, tensor rmin, int base, int big, int small) {
  int5 c = 0;
  float64 x = v_f32_ld_tnsr_b(c, in);
  for (int k = 0; k < 400; k++) {
    c[1] = k;
    v_f32_st_tnsr(c, bcast, v_broadcast_element_f32(x, k + base));
    v_f32_st_tnsr(c, shift, v_element_shift_up_f32(x, k + base));
  }
  c[1] = 400;
  v_f32_st_tnsr(c, bcast, v_broadcast_element_f32(x, big));
  v_f32_st_tnsr(c, shift, v_element_shift_up_f32(x, big));
  c[1] = 401;
  v_f32_st_tnsr(c, bcast, v_broadcast_element_f32(x, small));
  v_f32_st_tnsr(c, shift, v_element_shift_up_f32(x, small));
  c[1] = 402;
  v_f32_st_tnsr(c, bcast, v_broadcast_element_f32(x, 37));
  v_f32_st_tnsr(c, shift, v_element_shift_up_f32(x, 32));
  c[1] = 403;
  v_f32_st_tnsr(c, bcast, v_broadcast_element_f32(x, 0));
  v_f32_st_tnsr(c, shift, v_element_shift_up_f32(x, 0));
  for (int r = 0; r < 64; r++) {
    c[1] = r;
    v_i32_st_tnsr(c, rmin, v_i32_reduce_min(v_i32_ld_tnsr_b(c, in)));
  }
}

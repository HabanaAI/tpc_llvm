// Loads and stores through a global pointer that the instruction itself steps:
// st_g with inc_4/inc_8/inc_2, vector partial ld_g (steps by the element size)
// and int5 ld_g with an immediate and a register dimension mask (steps by 4).
//
// DIFFTEST: arch gaudi2 gaudi3
// DIFFTEST: in  i32 64 4 1 range=0,100000
// DIFFTEST: in  i32 64 1 1 range=31,31
// DIFFTEST: out i32 64 10 1
// DIFFTEST: index-space 1 1 1
void main(tensor in, tensor masks, tensor out) {
  int5 c = {0, 0, 0, 0, 0};

  // Row 0: ints at bytes 0, 8, 12, 20. Row 1: shorts at bytes 0, 2, 4.
  int __global *q = (int __global *)gen_addr(c, out, 0, 0, 1, 0);
  s_i32_st_g_inc(&q, 101, SW_INC_8, 1, 0);
  s_i32_st_g_inc(&q, 102, SW_INC_4, 1, 0);
  s_i32_st_g_inc(&q, 103, SW_INC_8, 1, 0);
  *q = 104;
  c[1] = 1;
  short __global *h = (short __global *)gen_addr(c, out, 0, 0, 1, 0);
  s_i16_st_g_inc(&h, 201, SW_INC_2, 1, 0);
  s_i16_st_g_inc(&h, 202, SW_INC_2, 1, 0);
  *h = 203;

  // Rows 2-3: two partial vector loads, the second 4 bytes after the first.
  int5 r = {0, 0, 0, 0, 0};
  void __global *a = gen_addr(r, in, 0, 0, 1, 0);
  int64 v1 = 0, v2 = 0;
  v1 = v_i32_ld_g_inc_partial_b(&a, 15, 0, 0, v1, 1, 0);
  v2 = v_i32_ld_g_inc_partial_b(&a, 15, 0, 0, v2, 1, 0);

  // Rows 4-9: int5 loads with an immediate mask, then a mask from a register.
  r[1] = 1;
  void __global *b = gen_addr(r, in, 0, 0, 1, 0);
  int5 x = 0, y = 0, z = 0;
  x = i_i32_ld_g_inc(&b, 0b00011, 0, x, 1, 0);
  y = i_i32_ld_g_inc(&b, 0b00011, 0, y, 1, 0);
  int5 m = {0, 0, 0, 0, 0};
  int mask = s_i32_ld_g((int __global *)gen_addr(m, masks, 0, 0, 1, 0), 0, 0, 1, 0);
  z = i_i32_ld_g_inc(&b, mask, 0, z, 1, 0);

  int5 o = {0, 2, 0, 0, 0};
  v_i32_st_tnsr(o, out, v1);
  o[1] = 3; v_i32_st_tnsr(o, out, v2);
  o[1] = 4; v_i32_st_tnsr(o, out, x[0]);
  o[1] = 5; v_i32_st_tnsr(o, out, x[1]);
  o[1] = 6; v_i32_st_tnsr(o, out, y[0]);
  o[1] = 7; v_i32_st_tnsr(o, out, y[1]);
  o[1] = 8; v_i32_st_tnsr(o, out, z[0]);
  o[1] = 9; v_i32_st_tnsr(o, out, z[4]);
}

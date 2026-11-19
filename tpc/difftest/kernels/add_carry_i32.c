// 64-bit adds from 32-bit ones: a low add, then a high add with SW_CARRY,
// which reads the carry of the add before it. Two independent pairs on the
// VPU and two on the SPU; each high add must read the carry of its own low
// add, so no other integer add of the same unit may come between them.
//
// DIFFTEST: arch gaudi gaudi2
// DIFFTEST: in  i32 64 8
// DIFFTEST: out i32 64 4
// DIFFTEST: out i32 64 4
// DIFFTEST: scalar i32 4294967295
// DIFFTEST: scalar i32 7
// DIFFTEST: scalar i32 1
// DIFFTEST: scalar i32 100
// DIFFTEST: scalar i32 4294967200
// DIFFTEST: scalar i32 9
// DIFFTEST: scalar i32 300
// DIFFTEST: scalar i32 1000

void main(tensor in, tensor vout, tensor sout, int a1lo, int a1hi, int b1lo,
          int b1hi, int a2lo, int a2hi, int b2lo, int b2hi) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 va1lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 va1hi = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 vb1lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 3; int64 vb1hi = v_i32_ld_tnsr_b(c, in);
  c[1] = 4; int64 va2lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 5; int64 va2hi = v_i32_ld_tnsr_b(c, in);
  c[1] = 6; int64 vb2lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 7; int64 vb2hi = v_i32_ld_tnsr_b(c, in);
  int64 v1lo = v_i32_add_b(va1lo, vb1lo, 0);
  int64 v1hi = v_i32_add_b(va1hi, vb1hi, SW_CARRY);
  int64 v2lo = v_i32_add_b(va2lo, vb2lo, 0);
  int64 v2hi = v_i32_add_b(va2hi, vb2hi, SW_CARRY);
  int s1lo = s_i32_add(a1lo, b1lo, 0);
  int s1hi = s_i32_add(a1hi, b1hi, SW_CARRY);
  int s2lo = s_i32_add(a2lo, b2lo, 0);
  int s2hi = s_i32_add(a2hi, b2hi, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, vout, v1lo);
  c[1] = 1; v_i32_st_tnsr(c, vout, v1hi);
  c[1] = 2; v_i32_st_tnsr(c, vout, v2lo);
  c[1] = 3; v_i32_st_tnsr(c, vout, v2hi);
  c[1] = 0; v_i32_st_tnsr(c, sout, s1lo);
  c[1] = 1; v_i32_st_tnsr(c, sout, s1hi);
  c[1] = 2; v_i32_st_tnsr(c, sout, s2lo);
  c[1] = 3; v_i32_st_tnsr(c, sout, s2hi);
}

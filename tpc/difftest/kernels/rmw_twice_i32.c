// The same RMW add twice must add twice: an RMW store reads the value it
// updates, so two identical ones are not one (EarlyCSE took them for one).
//
// DIFFTEST: arch gaudi gaudi2
// DIFFTEST: in  i32 64 2 range=-1000,1000
// DIFFTEST: out i32 64

void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  int64 x = v_i32_ld_tnsr_b(c, in);
  c[1] = 1;
  int64 y = v_i32_ld_tnsr_b(c, in);
  c[1] = 0;
  v_i32_st_tnsr(c, out, x);
  v_i32_st_tnsr_rmw(c, out, y, RMW_DT_INT32 | RMW_OP_ADD | RMW_SET, 0, 1, 0);
  v_i32_st_tnsr_rmw(c, out, y, RMW_DT_INT32 | RMW_OP_ADD | RMW_SET, 0, 1, 0);
}

// An RMW add in a loop with no other memory access must add on every
// iteration, so each element ends up n. LICM must not move it out of the loop.
// Not on gaudi3: both compilers store 1 there, reason not checked.
//
// DIFFTEST: arch gaudi gaudi2
// DIFFTEST: out i32 64
// DIFFTEST: scalar i32 5

void main(tensor out, int n) {
  int5 idx = {0, 0, 0, 0, 0};
  v_i32_st_tnsr(idx, out, 0);
  int64 one = 1;
  for (int i = 0; i < n; i++)
    v_i32_st_tnsr_rmw(idx, out, one, RMW_DT_INT32 | RMW_OP_ADD | RMW_SET, 0, 1, 0);
}

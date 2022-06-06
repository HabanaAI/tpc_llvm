// RUN: %tpc_clang -c -O1 %s -o %t.o -march=doron1
// RUN: %disasm --mcpu=doron1 %t.o | FileCheck --check-prefix=DIS %s

void 
main(int s0, tensor in, tensor out)
{
  int st;
  int5 cq = {0,0,0,0,0};
  float64 vec = s_f32_ld_g(gen_addr(cq, in, 0, 0, 1, 0));
  bool256 v_predicate = {0};

  reset_sqz_cntr(2, 0);

  v_f32_st_tnsr_sqz(cq, out, 2, vec, SW_CNT_ONLY, v_predicate, 0);
  v_f32_st_tnsr_sqz(cq, out, 2, vec, SW_FLUSH, v_predicate, 0);

}
// DIS:  st_l mmio vpu_only
// DIS:  st_tnsr_sqz cnt_only 0x2, VP 
// DIS:  st_tnsr_sqz flush 0x2

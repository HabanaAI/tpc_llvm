// RUN: %tpc_clang -c -O1 %s -o %t.o -march=gaudi2
// RUN: %disasm --mcpu=gaudi2 %t.o | FileCheck --check-prefix=DIS %s

// RUN: %tpc_clang -c -O1 %s -o %t.o -march=doron1
// RUN: %disasm --mcpu=doron1 %t.o | FileCheck --check-prefix=DIS %s

// XFAIL: *
// GAUDI-2175
void 
main(int s0, tensor out)
{
  int st;
  int5 cq = {0,0,0,0,0};
  half f = s_convert_i32_to_f16(s0,SW_CLIP_FP,0.0, 1,0);
  st = s_convert_i64_to_i32(0,s0,0,0,1,0);
  s_i32_st_g(gen_addr(cq, out, 0, 0, 1, 0), st, 0, 1, 0);
  cq[0]++;
  s_f16_st_g(gen_addr(cq, out, 0, 0, 1, 0), f, 0, 1, 0);

}
// DIS:  convert.i32 target_type=f16 rhne clip_fp
// DIS:  convert.i64 S

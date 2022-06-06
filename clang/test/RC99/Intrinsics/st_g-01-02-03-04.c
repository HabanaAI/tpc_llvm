// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi  -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -mllvm -tpc-inc-merger=0 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -mllvm -tpc-inc-merger=0 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -mllvm -tpc-inc-merger=0 -bfloat16 %s -o - | FileCheck %s

void main(tensor in, float xf, _Bool xb,
          int xi, short xs, char xc,
          unsigned int xui, unsigned short xus, unsigned char xuc,
          _Bool pred
          ) {
  int5 ndx = {0, 0, 0, 0, xi};
  void __global *ptr;

  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_f32_st_g(ptr, xf, 0, 1, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S0

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_i1_st_g(ptr, xb, 0, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %SP{{[0-9]+}}, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_i32_st_g(ptr, xi, 0, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S2, ![[Pred]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_u32_st_g(ptr, xui, 0, 0, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S5

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_i16_st_g(ptr, xs, 0, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S3, [[Pred]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_u16_st_g(ptr, xus, 0, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S6, ![[Pred]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_i8_st_g(ptr, xc, 0, 1, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S4

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  s_u8_st_g(ptr, xuc, 0, 1, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S7
}

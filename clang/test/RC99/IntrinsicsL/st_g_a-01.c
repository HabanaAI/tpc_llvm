// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(tensor in, float xf, _Bool xb,
          int xi, short xs, char xc,
          unsigned int xui, unsigned short xus, unsigned char xuc,
          _Bool pred
          ) {
  int5 ndx = {0, 0, 0, 0, xi};
  void __global *ptr;


  // F32
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  f32_st_g_a_s_b(ptr, xi, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S0, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  f32_st_g_a_s_b(ptr, xi, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S0, ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  f32_st_g_a_s(ptr, xi);
  // CHECK: st_g  %AD{{[0-9]}}, %S0, %SP0


  // BOOL
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  b_st_g_a_b_b(ptr, xb, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, [[XB:%SP[0-9]+]], [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  b_st_g_a_b_b(ptr, xb, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, [[XB]], ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  b_st_g_a_b(ptr, xb);
  // CHECK: st_g  %AD{{[0-9]}}, [[XB]], %SP0


  // I32
  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i32_st_g_a_s_b(ptr, xi, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S2, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i32_st_g_a_s_b(ptr, xi, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S2, ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i32_st_g_a_s(ptr, xi);
  // CHECK: st_g  %AD{{[0-9]}}, %S2, %SP0


  // U32
  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u32_st_g_a_s_b(ptr, xui, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S5, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u32_st_g_a_s_b(ptr, xui, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S5, ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u32_st_g_a_s(ptr, xui);
  // CHECK: st_g  %AD{{[0-9]}}, %S5, %SP0


  // I16
  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i16_st_g_a_s_b(ptr, xs, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S3, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i16_st_g_a_s_b(ptr, xs, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S3, ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i16_st_g_a_s(ptr, xs);
  // CHECK: st_g  %AD{{[0-9]}}, %S3, %SP0


  // U16
  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u16_st_g_a_s_b(ptr, xus, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S6, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u16_st_g_a_s_b(ptr, xus, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S6, ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u16_st_g_a_s(ptr, xus);
  // CHECK: st_g  %AD{{[0-9]}}, %S6, %SP0


  // I8
  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i8_st_g_a_s_b(ptr, xc, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S4, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i8_st_g_a_s_b(ptr, xc, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S4, ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  i8_st_g_a_s(ptr, xc);
  // CHECK: st_g  %AD{{[0-9]}}, %S4, %SP0


  // U8
  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u8_st_g_a_s_b(ptr, xuc, pred, 0);
  // CHECK: st_g  %AD{{[0-9]}}, %S7, [[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u8_st_g_a_s_b(ptr, xuc, pred, 1);
  // CHECK: st_g  %AD{{[0-9]}}, %S7, ![[Pred:%SP[0-9]+]]

  ndx[0]++;
  ptr = gen_addr(ndx, in, 0, 0, 1, 0);
  u8_st_g_a_s(ptr, xuc);
  // CHECK: st_g  %AD{{[0-9]}}, %S7, %SP0
}

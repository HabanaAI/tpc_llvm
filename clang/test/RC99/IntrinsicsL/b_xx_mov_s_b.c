// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI %s

void main(int dest, int src, int src1) {
  int __local *dptr = (int  __local *)dest;

  _Bool res = *dptr++ & 0x01;
  _Bool pred = src > src1;
// CHECK-DAG: ld_l         [[RESV:%S[0-9]+]], %S0
// CHECK-DAG: mov          [[RES:%SP[0-9]+]], [[RESV]]
// CHECK-DAG: cmp_grt.i32  [[PRED:%SP[0-9]+]], %S1, %S2

  float xf = as_float(*dptr++);
  res = b_f32_mov_s_b(xf, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]

#ifdef __gaudi__
  _BFloat16 xbf = as_bfloat((short)*dptr++);
  res = b_bf16_mov_s_b(xbf, res, pred, 0);
  *dptr++ = res;
// GAUDI: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]
#endif

  int xi = *dptr++;
  res = b_i32_mov_s_b(xi, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]

  unsigned xu = *dptr++;
  res = b_u32_mov_s_b(xu, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]

  short xs = *dptr++;
  res = b_i16_mov_s_b(xs, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]

  unsigned short xus = *dptr++;
  res = b_u16_mov_s_b(xus, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]

  char xc = *dptr++;
  res = b_i8_mov_s_b(xc, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]

  unsigned char xuc = *dptr++;
  res = b_u8_mov_s_b(xuc, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], %S{{[0-9]+}}, [[PRED]]
}

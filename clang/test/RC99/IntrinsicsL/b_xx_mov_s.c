// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI %s

void main(int dest, int src, _Bool pred) {
  int __local *dptr = (int  __local *)dest;
  _Bool res;

  float xf = as_float(*dptr++);
  res = b_f32_mov_s(xf);
  *dptr++ = res;
// CHECK: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}

#ifdef __gaudi__
  _BFloat16 xbf = as_bfloat((short)*dptr++);
  res = b_bf16_mov_s(xbf);
  *dptr++ = res;
// GAUDI: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}
#endif

  int xi = *dptr++;
  res = b_i32_mov_s(xi);
  *dptr++ = res;
// CHECK: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}

  unsigned xu = *dptr++;
  res = b_u32_mov_s(xu);
  *dptr++ = res;
// CHECK: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}

  short xs = *dptr++;
  res = b_i16_mov_s(xs);
  *dptr++ = res;
// CHECK: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}

  unsigned short xus = *dptr++;
  res = b_u16_mov_s(xus);
  *dptr++ = res;
// CHECK: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}

  char xc = *dptr++;
  res = b_i8_mov_s(xc);
  *dptr++ = res;
// CHECK: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}

  unsigned char xuc = *dptr++;
  res = b_u8_mov_s(xuc);
  *dptr++ = res;
// CHECK: mov  %SP{{[0-9]+}}, %S{{[0-9]+}}
}

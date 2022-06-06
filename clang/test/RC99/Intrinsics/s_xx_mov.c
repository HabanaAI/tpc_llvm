// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s


void main(int dest, float xf, int xi, short xs, char xc) {
  _Bool pred = xi < dest;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S2, %S0

  volatile int __local *dptr = (int __local *)dest;

  // s_f32_mov
  {
    float res = as_float(*dptr++);
    // CHECK-DAG: ld_l [[RES:%S[0-9]+]], %S0
    
    res = s_f32_mov(xf, 0, res, pred, 0);
    // CHECK: mov.f32  [[RES]], %S1, [[PRED]]
    *dptr++ = as_int(res);
    
    res = s_f32_mov(1.5, 0, res, pred, 0);
    // CHECK: mov.f32  [[RES]], 0x3fc00000, [[PRED]]
    *dptr++ = as_int(res);
  }

#if defined(__gaudi__) || defined(__goya2__)
  // s_bf16_mov
  {
    _BFloat16 res = as_bfloat((unsigned short)*dptr++);
    
    res = s_bf16_mov(as_bfloat(xs), 0, res, pred, 0);
    // GEN2: mov.bf16  [[RES:%S[0-9]+]], %S{{[0-9]+}}, [[PRED]]
    *dptr++ = as_ushort(res);
    
    res = s_bf16_mov(1.5, 0, res, pred, 0);
    // GEN2: mov.bf16  [[RES]], 0x3fc0, [[PRED]]
    *dptr++ = as_ushort(res);
  }
#endif  

#if defined(__goya2__)
  // s_f16_mov
  {
    half res = as_half((unsigned short)*dptr++);
    
    res = s_f16_mov(as_half(xs), 0, res, pred, 0);
    // GEN3: mov.f16  [[RES:%S[0-9]+]], %S{{[0-9]+}}, [[PRED]]
    *dptr++ = as_ushort(res);
    
    res = s_f16_mov(1.0, 0, res, pred, 0);
    // GEN3: mov.f16  [[RES]], 0x3c00, [[PRED]]
    *dptr++ = as_ushort(res);
  }
#endif  

  // s_i32_mov
  {
    int res = as_int(*dptr++);
    // CHECK: ld_l [[RES:%S[0-9]+]], %S{{[0-9]+}}
    
    res = s_i32_mov(xi, 0, res, pred, 0);
    // CHECK: mov.i32  [[RES]], %S2, [[PRED]]
    *dptr++ = as_int(res);
    
    res = s_i32_mov(123, 0, res, pred, 0);
    // CHECK: mov.i32  [[RES]], 0x7b, [[PRED]]
    *dptr++ = as_int(res);
  }

  // s_u32_mov
  {
    int res = as_uint(*dptr++);
    // CHECK: ld_l [[RES:%S[0-9]+]], %S{{[0-9]+}}
    
    res = s_u32_mov(as_uint(xi), 0, res, pred, 0);
    // CHECK: mov.u32  [[RES]], %S2, [[PRED]]
    *dptr++ = as_int(res);
    
    res = s_u32_mov(123, 0, res, pred, 0);
    // CHECK: mov.u32  [[RES]], 0x7b, [[PRED]]
    *dptr++ = as_int(res);
  }

  // s_i16_mov
  {
    short res = (short)*dptr++;
    
    res = s_i16_mov(xs, 0, res, pred, 0);
    // CHECK: mov.i16  [[RES:%S[0-9]+]], %S3, [[PRED]]
    *dptr++ = (int)res;
    
    res = s_i16_mov(123, 0, res, pred, 0);
    // CHECK: mov.i16  [[RES]], 0x7b, [[PRED]]
    *dptr++ = (int)res;
  }

  // s_u16_mov
  {
    unsigned short res = (unsigned short)*dptr++;
    
    res = s_u16_mov((unsigned short)xs, 0, res, pred, 0);
    // CHECK: mov.u16  [[RES:%S[0-9]+]], %S3, [[PRED]]
    *dptr++ = (int)res;
    
    res = s_u16_mov(123, 0, res, pred, 0);
    // CHECK: mov.u16  [[RES]], 0x7b, [[PRED]]
    *dptr++ = (int)res;
  }

  // s_i8_mov
  {
    char res = (char)*dptr++;
    
    res = s_i8_mov(xc, 0, res, pred, 0);
    // CHECK: mov.i8  [[RES:%S[0-9]+]], %S4, [[PRED]]
    *dptr++ = (int)res;
    
    res = s_i8_mov(123, 0, res, pred, 0);
    // CHECK: mov.i8  [[RES]], 0x7b, [[PRED]]
    *dptr++ = (int)res;
  }

  // s_u8_mov
  {
    char res = (unsigned char)*dptr++;
    
    res = s_u8_mov((unsigned char)xc, 0, res, pred, 0);
    // CHECK: mov.u8  [[RES:%S[0-9]+]], %S4, [[PRED]]
    *dptr++ = (int)res;
    
    res = s_u8_mov(123, 0, res, pred, 0);
    // CHECK: mov.u8  [[RES]], 0x7b, [[PRED]]
    *dptr++ = (int)res;
  }
}

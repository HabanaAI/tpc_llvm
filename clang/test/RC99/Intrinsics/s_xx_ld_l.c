// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s

void main(unsigned addr, unsigned addr1, int dest, float xf) {
  volatile int __local *dptr = (int __local *)dest;
  _Bool pred = (addr < addr1);
  // CHECK: cmp_less.u32 [[PRED:%SP[0-9]+]], %S0, %S1

  // s_f32_ld_l
  {
    float res = as_float(*dptr++);
    res = s_f32_ld_l(addr, 0, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l [[VAL:%S[0-9]+]], %S0, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_f32_ld_l(0x100, 0, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l [[VAL]], 0x100, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

#if defined(__gaudi__) || defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  // s_bf16_ld_l
  {
    _BFloat16 res = as_bfloat((short)*dptr++);
    res = s_bf16_ld_l(addr, 0, res, pred, 0);
    *(_BFloat16 __local *)dptr = res;
    ++dptr;
    // GEN2: ld_l [[VAL:%S[0-9]+]], %S0, [[PRED]]
    // GEN2: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_bf16_ld_l(0x200, 0, res, pred, 0);
    *(_BFloat16 __local *)dptr = res;
    ++dptr;
    // GEN2: ld_l [[VAL:%S[0-9]+]], 0x200, [[PRED]]
    // GEN2: st_l %S{{[0-9]+}}, [[VAL]]
  }
#endif

#if defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  // s_f16_ld_l
  {
    half res = as_half((short)*dptr++);
    res = s_f16_ld_l(addr, 0, res, pred, 0);
    *(half __local *)dptr = res;
    ++dptr;
    // GEN3: ld_l [[VAL:%S[0-9]+]], %S0, [[PRED]]
    // GEN3: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_f16_ld_l(0x300, 0, res, pred, 0);
    *(half __local *)dptr = res;
    ++dptr;
    // GEN3: ld_l [[VAL:%S[0-9]+]], 0x300, [[PRED]]
    // GEN3: st_l %S{{[0-9]+}}, [[VAL]]
  }
#endif

  // s_i32_ld_l
  {
    int res = as_int(*dptr++);
    res = s_i32_ld_l(addr1, 0, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l [[VAL:%S[0-9]+]], %S1, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_i32_ld_l(0x400, 0, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l [[VAL:%S[0-9]+]], 0x400, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

  // s_u32_ld_l
  {
    unsigned res = as_uint(*dptr++);
    res = s_u32_ld_l(addr1, 0, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l [[VAL:%S[0-9]+]], %S1, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]

    res = s_u32_ld_l(0x500, 0, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l [[VAL:%S[0-9]+]], 0x500, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

  // s_i16_ld_l
  {
    short res = *dptr++;
    res = s_i16_ld_l(addr, 0, res, pred, 0);
    *(short __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], %S0, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_i16_ld_l(0x600, 0, res, pred, 0);
    *(short __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], 0x600, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

  // s_u16_ld_l
  {
    unsigned short res = *dptr++;
    res = s_u16_ld_l(addr, 0, res, pred, 0);
    *(unsigned short __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], %S0, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_u16_ld_l(0x700, 0, res, pred, 0);
    *(unsigned short __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], 0x700, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

  // s_i8_ld_l
  {
    char res = *dptr++;
    res = s_i8_ld_l(addr1, 0, res, pred, 0);
    *(char __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], %S1, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_i8_ld_l(0x800, 0, res, pred, 0);
    *(char __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], 0x800, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

  // s_u8_ld_l
  {
    unsigned char res = *dptr++;
    res = s_u8_ld_l(addr1, 0, res, pred, 0);
    *(unsigned char __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], %S1, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_u8_ld_l(0x900, 0, res, pred, 0);
    *(unsigned char __local *)dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%S[0-9]+]], 0x900, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

  // s_i1_ld_l
  {
    _Bool res = dest < addr1;
    res = s_i1_ld_l(addr, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: ld_l [[VAL:%SP[0-9]+]], %S0, [[PRED]]
    // CHECK: mov.i32  [[IVAL:%S[0-9]+]], 0x1, [[VAL]]
    // CHECK: mov.i32  [[IVAL]], 0x0, ![[VAL]]
    // CHECK: st_l %S{{[0-9]+}}, [[IVAL]]
    
    res = s_i1_ld_l(0x980, 0, res, pred, 0);
    *dptr = res;
    ++dptr;
    // CHECK: ld_l [[VAL:%SP[0-9]+]], 0x980, [[PRED]]
    // CHECK: mov.i32  [[IVAL:%S[0-9]+]], 0x1, [[VAL]]
    // CHECK: mov.i32  [[IVAL]], 0x0, ![[VAL]]
    // CHECK: st_l %S{{[0-9]+}}, [[IVAL]]
  }

  // s_i32_ld_l + MMIO
  {
    int res = as_int(*dptr++);
    res = s_i32_ld_l(addr1, SW_MMIO, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l mmio [[VAL:%S[0-9]+]], %S1, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_i32_ld_l(0xa00, SW_MMIO, res, pred, 0);
    *dptr++ = as_int(res);
    // CHECK: ld_l mmio [[VAL:%S[0-9]+]], 0xa00, [[PRED]]
    // CHECK: st_l %S{{[0-9]+}}, [[VAL]]
  }

#if defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  // s_i32_ld_l + LOCK
  {
    int res = as_int(*dptr++);
    res = s_i32_ld_l(addr1, SW_MMIO | SW_LOCK, res, pred, 0);
    *dptr++ = as_int(res);
    // GEN3: ld_l mmio lock [[VAL:%S[0-9]+]], %S1, [[PRED]]
    // GEN3`: st_l %S{{[0-9]+}}, [[VAL]]
    
    res = s_i32_ld_l(0xb00, SW_LOCK, res, pred, 0);
    *dptr++ = as_int(res);
    // GEN3: ld_l lock [[VAL:%S[0-9]+]], 0xb00, [[PRED]]
    // GEN3: st_l %S{{[0-9]+}}, [[VAL]]
  }
#endif
}

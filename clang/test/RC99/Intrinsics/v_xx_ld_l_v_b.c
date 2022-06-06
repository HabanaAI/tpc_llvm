// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck  --check-prefix=CHECK-GEN1 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefix=CHECK-GEN3 %s

void main(unsigned addr0, unsigned addr1, unsigned addr2, int dest, _Bool pred) {
  int64 __local *vptr = (int64 __local *)dest;
  
  // v_f32_ld_l_v_b
  {
    float64 __local *dptr = (float64 __local *)vptr;
    float64 result = *dptr++;

    result = v_f32_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_f32_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_f32_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_f32_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_f32_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_f32_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }

  // v_bf16_ld_l_v_b
#if defined(__gaudi__) || defined(__goya2)
  {
    bfloat128 __local *dptr = (bfloat128 __local *)vptr;
    bfloat128 result = 0;

    result = v_bf16_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_bf16_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_bf16_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_bf16_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_bf16_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_bf16_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
#endif

  // v_f16_ld_l_v_b
#if defined(__goya2)
  {
    half128 __local *dptr = (half128 __local *)vptr;
    half128 result = 0;

    result = v_f16_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN3:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_f16_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN3:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_f16_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN3:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_f16_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN3:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_f16_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN3:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_f16_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN3:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
#endif

  // v_i32_ld_l_v_b
  {
    int64 __local *dptr = (int64 __local *)vptr;
    int64 result = 0;

    result = v_i32_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_i32_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_i32_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_i32_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_i32_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_i32_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_u32_ld_l_v_b
  {
      uint64 __local *dptr = (uint64 __local *)dest;
    uint64 result = 0;

    result = v_u32_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_u32_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_u32_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_u32_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_u32_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_u32_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_i16_ld_l_v_b
  {
    short128 __local *dptr = (short128 __local *)vptr;
    short128 result = 0;

    result = v_i16_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_i16_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_i16_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_i16_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_i16_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_i16_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_u16_ld_l_v_b
  {
    ushort128 __local *dptr = (ushort128 __local *)vptr;
    ushort128 result = 0;

    result = v_u16_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_u16_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_u16_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_u16_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_u16_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_u16_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_i8_ld_l_v_b
  {
    char256 __local *dptr = (char256 __local *)vptr;
    char256 result = 0;

    result = v_i8_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_i8_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_i8_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_i8_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_i8_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_i8_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }

  // v_u8_ld_l_v_b
  {
    uchar256 __local *dptr = (uchar256 __local *)vptr;
    uchar256 result = 0;

    result = v_u8_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES:[0-9]+]], %S0
    *dptr++ = result;

    result = v_u8_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S1, %SP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_u8_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], %S2, !%SP[[PRED]]
    *dptr++ = result;
    
    result = v_u8_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x100
    *dptr++ = result;
    
    result = v_u8_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x200, %SP[[PRED]]
    *dptr++ = result;
    
    result = v_u8_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %V[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %V[[RES]], 0x300, !%SP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_i1_ld_l_v_b
  {
    bool256 __local *dptr = (bool256 __local *)dest;
    bool256 result = 0;

    result = v_i1_ld_l_v_b(addr0, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %VP[[RES:[0-9]+]], %S0, 0x0
    // CHECK-GEN2:     ld_l_v  %VP[[RES:[0-9]+]], %S0

    *dptr++ = result;

    result = v_i1_ld_l_v_b(addr1, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %VP[[RES]], %S1, 0x0, %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     ld_l_v  %VP[[RES]], %S1, %SP[[PRED:[0-9]+]]

    *dptr++ = result;

    result = v_i1_ld_l_v_b(addr2, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %VP[[RES]], %S2, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %VP[[RES]], %S2, !%SP[[PRED]]

    *dptr++ = result;
    
    result = v_i1_ld_l_v_b(0x100, 0, result, 1, 0);
    // CHECK-GEN1:     ld_l_v  %VP[[RES]], %S{{[0-9]+}}, 0x0
    // CHECK-GEN2:     ld_l_v  %VP[[RES]], 0x100

    *dptr++ = result;
    
    result = v_i1_ld_l_v_b(0x200, 0, result, pred, 0);
    // CHECK-GEN1:     ld_l_v  %VP[[RES]], %S{{[0-9]+}}, 0x0, %SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %VP[[RES]], 0x200, %SP[[PRED]]

    *dptr++ = result;
    
    result = v_i1_ld_l_v_b(0x300, 0, result, pred, 1);
    // CHECK-GEN1:     ld_l_v  %VP[[RES]], %S{{[0-9]+}}, 0x0, !%SP[[PRED]]
    // CHECK-GEN2:     ld_l_v  %VP[[RES]], 0x300, !%SP[[PRED]]

    *dptr++ = result;
  }
}

// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck  --check-prefix=CHECK-GEN1 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefix=CHECK-GEN3 %s

void main(unsigned addr0, unsigned addr1, unsigned addr2, int dest, _Bool pred) {
  volatile int64 __local *vptr = (int64 __local *)dest;
  
  // v_f32_st_l_v_low
  {
    float64 __local *sptr = (float64 __local *)vptr;
    float64 value = *sptr++;

    v_f32_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %V[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_f32_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %V[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_f32_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_f32_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_f32_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_f32_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }

#if defined(__gaudi__) || defined(__goya2__)
  // v_bf16_st_l_v_low
  {
    bfloat128 __local *sptr = (bfloat128 __local *)vptr;
    bfloat128 value = *sptr++;

    v_bf16_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_bf16_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_bf16_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_bf16_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_bf16_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_bf16_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
#endif

#if defined(__goya2__)
  // v_f16_st_l_v_low
  {
    half128 __local *sptr = (half128 __local *)vptr;
    half128 value = *sptr++;

    v_f16_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN3:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_f16_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN3:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_f16_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN3:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_f16_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN3:     st_l_v_low  0x100, %V[[RES]]
    
    v_f16_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN3:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_f16_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN3:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
#endif
  
  // v_i32_st_l_v_low
  {
    int64 __local *sptr = (int64 __local *)vptr;
    int64 value = *sptr++;

    v_i32_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %V[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_i32_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %V[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_i32_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_i32_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_i32_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_i32_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
  
  // v_u32_st_l_v_low
  {
    uint64 __local *sptr = (uint64 __local *)vptr;
    uint64 value = *sptr++;

    v_u32_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %V[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_u32_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %V[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_u32_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_u32_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_u32_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_u32_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
  
  // v_i16_st_l_v_low
  {
    short128 __local *sptr = (short128 __local *)vptr;
    short128 value = *sptr++;

    v_i16_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %V[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_i16_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %V[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_i16_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_i16_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_i16_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_i16_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
  
  // v_u16_st_l_v_low
  {
    ushort128 __local *sptr = (ushort128 __local *)vptr;
    ushort128 value = *sptr++;

    v_u16_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %V[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_u16_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %V[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_u16_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_u16_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_u16_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_u16_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
  
  // v_i8_st_l_v_low
  {
    char256 __local *sptr = (char256 __local *)vptr;
    char256 value = *sptr++;

    v_i8_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %V[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_i8_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %V[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_i8_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_i8_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_i8_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_i8_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
  
  // v_u8_st_l_v_low
  {
    uchar256 __local *sptr = (uchar256 __local *)vptr;
    uchar256 value = *sptr++;

    v_u8_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %V[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %V[[RES:[0-9]+]]

    v_u8_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %V[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %V[[RES]], %SP[[PRED:[0-9]+]]

    v_u8_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %V[[RES]], !%SP[[PRED]]
    
    v_u8_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %V[[RES]]
    
    v_u8_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %V[[RES]], %SP[[PRED]]
    
    v_u8_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %V[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %V[[RES]], !%SP[[PRED]]
  }
  
  // v_i1_st_l_v_low
  {
    bool256 __local *sptr = (bool256 __local *)vptr;
    bool256 value = *sptr++;

    v_i1_st_l_v_low(addr0, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S0, 0x0, %VP[[RES:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S0, %VP[[RES:[0-9]+]]

    v_i1_st_l_v_low(addr1, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S1, 0x0, %VP[[RES]], %SP[[PRED:[0-9]+]]
    // CHECK-GEN2:     st_l_v_low  %S1, %VP[[RES]], %SP[[PRED:[0-9]+]]

    v_i1_st_l_v_low(addr2, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S2, 0x0, %VP[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  %S2, %VP[[RES]], !%SP[[PRED]]
    
    v_i1_st_l_v_low(0x100, value, 0, 1, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %VP[[RES]]
    // CHECK-GEN2:     st_l_v_low  0x100, %VP[[RES]]
    
    v_i1_st_l_v_low(0x200, value, 0, pred, 0);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %VP[[RES]], %SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x200, %VP[[RES]], %SP[[PRED]]
    
    v_i1_st_l_v_low(0x300, value, 0, pred, 1);
    // CHECK-GEN1:     st_l_v_low  %S{{[0-9]+}}, 0x0, %VP[[RES]], !%SP[[PRED]]
    // CHECK-GEN2:     st_l_v_low  0x300, %VP[[RES]], !%SP[[PRED]]
  }
}

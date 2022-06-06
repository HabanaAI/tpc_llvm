// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-GEN2,CHECK-GEN3 %s


void main(unsigned addr0, unsigned addr1, unsigned addr2, int dest, int pred_ptr) {
  int64 __local *vptr = (int64 __local *)dest;
  bool256 pred = *(bool256 __local *)pred_ptr;
  
  // v_f32_ld_l_v_vb
  {
    *vptr++ = 111;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x6f
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    float64 __local *dptr = (float64 __local *)vptr;
    float64 result = *dptr++;
    bool64 vpred = to_bool64(pred);

    result = v_f32_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES:[0-9]+]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_f32_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_f32_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_f32_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_bf16_ld_l_v_low_vb
  {
    *vptr++ = 112;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x70
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    bfloat128 __local *dptr = (bfloat128 __local *)vptr;
    bfloat128 result = *dptr++;
    bool128 vpred = to_bool128(pred);

    result = v_bf16_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_bf16_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_bf16_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_bf16_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
# if defined(__goya2__)  
  // v_f16_ld_l_v_low_vb
  {
    *vptr++ = 113;
    // CHECK-GEN3: mov.i32 [[MARK:%V[0-9]+]], 0x71
    // CHECK-GEN3: st_l_v  %S{{[0-9]+}}, [[MARK]]

    half128 __local *dptr = (half128 __local *)vptr;
    half128 result = *dptr++;
    bool128 vpred = to_bool128(pred);

    result = v_f16_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN3:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_f16_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN3:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_f16_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN3:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_f16_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN3:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
#endif
  
  // v_i32_ld_l_v_low_vb
  {
    *vptr++ = 114;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x72
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    int64 __local *dptr = (int64 __local *)vptr;
    int64 result = *dptr++;
    bool64 vpred = to_bool64(pred);

    result = v_i32_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_i32_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_i32_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_i32_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_u32_ld_l_v_low_vb
  {
    *vptr++ = 115;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x73
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    uint64 __local *dptr = (uint64 __local *)vptr;
    uint64 result = *dptr++;
    bool64 vpred = to_bool64(pred);

    result = v_u32_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_u32_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_u32_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_u32_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_i16_ld_l_v_low_vb
  {
    *vptr++ = 116;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x74
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    short128 __local *dptr = (short128 __local *)vptr;
    short128 result = *dptr++;
    bool128 vpred = to_bool128(pred);

    result = v_i16_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_i16_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_i16_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_i16_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_u16_ld_l_v_low_vb
  {
    *vptr++ = 117;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x75
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    ushort128 __local *dptr = (ushort128 __local *)vptr;
    ushort128 result = *dptr++;
    bool128 vpred = to_bool128(pred);

    result = v_u16_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_u16_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_u16_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_u16_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_i8_ld_l_v_low_vb
  {
    *vptr++ = 118;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x76
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    char256 __local *dptr = (char256 __local *)vptr;
    char256 result = *dptr++;
    bool256 vpred = pred;

    result = v_i8_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_i8_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_i8_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_i8_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_u8_ld_l_v_low_vb
  {
    *vptr++ = 119;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x77
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    uchar256 __local *dptr = (uchar256 __local *)vptr;
    uchar256 result = *dptr++;
    bool256 vpred = pred;

    result = v_u8_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_u8_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_u8_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_u8_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %V[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;

    vptr = (int64 __local *)dptr;
  }
  
  // v_i1_ld_l_v_low_vb
  {
    *vptr++ = 120;
    // CHECK-GEN2: mov.i32 [[MARK:%V[0-9]+]], 0x78
    // CHECK-GEN2: st_l_v  %S{{[0-9]+}}, [[MARK]]

    bool256 __local *dptr = (bool256 __local *)vptr;
    bool256 result = *dptr++;
    bool256 vpred = pred;

    result = v_i1_ld_l_v_low_vb(addr1, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %VP[[RES:[0-9]+]], %S1, %VP[[PRED:[0-9]+]]
    *dptr++ = result;

    result = v_i1_ld_l_v_low_vb(addr2, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %VP[[RES]], %S2, !%VP[[PRED]]
    *dptr++ = result;
    
    result = v_i1_ld_l_v_low_vb(0x200, 0, result, vpred, 0);
    // CHECK-GEN2:     ld_l_v_low  %VP[[RES]], 0x200, %VP[[PRED]]
    *dptr++ = result;
    
    result = v_i1_ld_l_v_low_vb(0x300, 0, result, vpred, 1);
    // CHECK-GEN2:     ld_l_v_low  %VP[[RES]], 0x300, !%VP[[PRED]]
    *dptr++ = result;
  }

}

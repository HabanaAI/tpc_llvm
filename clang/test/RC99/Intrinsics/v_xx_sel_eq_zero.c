
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s
void main(int dest, int src1, int src2, int pred_val, int vect_pred,
          float af32, int ai32, unsigned au32, short ai16, unsigned short au16,
          char ai8, unsigned char au8) {
  int64 __local *dest_ptr = (int64 __local *)dest;          
  int64 __local *src_ptr = (int64 __local *)src1;          
  int64 __local *res_ptr = (int64 __local *)src2;
  bool256 __local *vpred_ptr = (bool256 __local *)vect_pred;
  #if defined(__gaudi__) || defined(__goya2__)
  _BFloat16 abf16 = as_bf16(ai16);
  #endif
  #if defined(__goya2__)
  half af16= as_half(au16);
  #endif
  
  _Bool spred = src1 < src2; 
  bool256 vpred = *vpred_ptr;
  // CHECK-DAG: cmp_less.i32 [[SPRED:%SP[0-9]+]], %S1, %S2
  // CHECK-DAG: ld_l_v [[VPRED:%VP[0-9]+]]
 
  {
    volatile float64 __local *dptr = (float64 __local *)dest_ptr;
    volatile float64 __local *sptr = (float64 __local *)src_ptr;
    volatile float64 __local *rptr = (float64 __local *)res_ptr;
    
    float64 result = *dptr++;
    float64 x1 = *sptr++;
    float64 x2 = *sptr++;
    float64 x3 = *rptr++;
    float64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_f32_sel_eq_f32_b
    
    result = v_f32_sel_eq_f32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f32_sel_eq_f32_b(x1, af32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f32_sel_eq_f32_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f32_sel_eq_f32_b(x1, x2, x3, af32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[SPRED]]     
    
    result = v_f32_sel_eq_f32_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[SPRED]]

    //------ v_f32_sel_eq_f32_vb
    
    result = v_f32_sel_eq_f32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f32_sel_eq_f32_vb(x1, af32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f32_sel_eq_f32_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f32_sel_eq_f32_vb(x1, x2, x3, af32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[VPRED]]     
    
    result = v_f32_sel_eq_f32_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile float64 __local *dptr = (float64 __local *)dest_ptr;
    volatile int64 __local *sptr = (int64 __local *)src_ptr;
    volatile float64 __local *rptr = (float64 __local *)res_ptr;
    
    float64 result = *dptr++;
    int64 x1 = *sptr++;
    int64 x2 = *sptr++;
    float64 x3 = *rptr++;
    float64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_f32_sel_eq_i32_b
    
    result = v_f32_sel_eq_i32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f32_sel_eq_i32_b(x1, ai32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f32_sel_eq_i32_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f32_sel_eq_i32_b(x1, x2, x3, af32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[SPRED]]     
    
    result = v_f32_sel_eq_i32_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[SPRED]]

    //------ v_f32_sel_eq_i32_vb
    
    result = v_f32_sel_eq_i32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f32_sel_eq_i32_vb(x1, ai32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f32_sel_eq_i32_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f32_sel_eq_i32_vb(x1, x2, x3, af32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[VPRED]]     
    
    result = v_f32_sel_eq_i32_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile float64 __local *dptr = (float64 __local *)dest_ptr;
    volatile uint64 __local *sptr = (uint64 __local *)src_ptr;
    volatile float64 __local *rptr = (float64 __local *)res_ptr;
    
    float64 result = *dptr++;
    uint64 x1 = *sptr++;
    uint64 x2 = *sptr++;
    float64 x3 = *rptr++;
    float64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_f32_sel_eq_u32_b
    
    result = v_f32_sel_eq_u32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f32_sel_eq_u32_b(x1, au32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f32_sel_eq_u32_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f32_sel_eq_u32_b(x1, x2, x3, af32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[SPRED]]     
    
    result = v_f32_sel_eq_u32_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[SPRED]]

    //------ v_f32_sel_eq_u32_vb
    
    result = v_f32_sel_eq_u32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f32_sel_eq_u32_vb(x1, au32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f32_sel_eq_u32_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f32_sel_eq_u32_vb(x1, x2, x3, af32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[VPRED]]     
    
    result = v_f32_sel_eq_u32_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

#if defined (__gaudi__) || defined(__goya2__)
     
  {
    volatile bfloat128 __local *dptr = (bfloat128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;
    
    bfloat128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN2: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_bf16_sel_eq_bf16_b
    
    result = v_bf16_sel_eq_bf16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_bf16_sel_eq_bf16_b(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_bf16_sel_eq_bf16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_bf16_sel_eq_bf16_b(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_bf16_sel_eq_bf16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel_eq_bf16_vb
    
    result = v_bf16_sel_eq_bf16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_bf16_sel_eq_bf16_vb(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_bf16_sel_eq_bf16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_bf16_sel_eq_bf16_vb(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_bf16_sel_eq_bf16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
     
  {
    volatile bfloat128 __local *dptr = (bfloat128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;
    
    bfloat128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN3: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_bf16_sel_eq_f16_b
    
    result = v_bf16_sel_eq_f16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_bf16_sel_eq_f16_b(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_bf16_sel_eq_f16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_bf16_sel_eq_f16_b(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_bf16_sel_eq_f16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel_eq_f16_vb
    
    result = v_bf16_sel_eq_f16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_bf16_sel_eq_f16_vb(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_bf16_sel_eq_f16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_bf16_sel_eq_f16_vb(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_bf16_sel_eq_f16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined (__gaudi__) || defined(__goya2__)
     
  {
    volatile bfloat128 __local *dptr = (bfloat128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;
    
    bfloat128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN2: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_bf16_sel_eq_i16_b
    
    result = v_bf16_sel_eq_i16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_bf16_sel_eq_i16_b(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_bf16_sel_eq_i16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_bf16_sel_eq_i16_b(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_bf16_sel_eq_i16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel_eq_i16_vb
    
    result = v_bf16_sel_eq_i16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_bf16_sel_eq_i16_vb(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_bf16_sel_eq_i16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_bf16_sel_eq_i16_vb(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_bf16_sel_eq_i16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined (__gaudi__) || defined(__goya2__)
     
  {
    volatile bfloat128 __local *dptr = (bfloat128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;
    
    bfloat128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN2: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_bf16_sel_eq_u16_b
    
    result = v_bf16_sel_eq_u16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_bf16_sel_eq_u16_b(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_bf16_sel_eq_u16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_bf16_sel_eq_u16_b(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_bf16_sel_eq_u16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel_eq_u16_vb
    
    result = v_bf16_sel_eq_u16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_bf16_sel_eq_u16_vb(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_bf16_sel_eq_u16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_bf16_sel_eq_u16_vb(x1, x2, x3, abf16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_bf16_sel_eq_u16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
     
  {
    volatile half128 __local *dptr = (half128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;
    
    half128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN3: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_f16_sel_eq_f16_b
    
    result = v_f16_sel_eq_f16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f16_sel_eq_f16_b(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f16_sel_eq_f16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f16_sel_eq_f16_b(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_f16_sel_eq_f16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel_eq_f16_vb
    
    result = v_f16_sel_eq_f16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f16_sel_eq_f16_vb(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f16_sel_eq_f16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f16_sel_eq_f16_vb(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_f16_sel_eq_f16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
     
  {
    volatile half128 __local *dptr = (half128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;
    
    half128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN3: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_f16_sel_eq_bf16_b
    
    result = v_f16_sel_eq_bf16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f16_sel_eq_bf16_b(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f16_sel_eq_bf16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f16_sel_eq_bf16_b(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_f16_sel_eq_bf16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel_eq_bf16_vb
    
    result = v_f16_sel_eq_bf16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f16_sel_eq_bf16_vb(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f16_sel_eq_bf16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f16_sel_eq_bf16_vb(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_f16_sel_eq_bf16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
     
  {
    volatile half128 __local *dptr = (half128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;
    
    half128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN3: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_f16_sel_eq_i16_b
    
    result = v_f16_sel_eq_i16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f16_sel_eq_i16_b(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f16_sel_eq_i16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f16_sel_eq_i16_b(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_f16_sel_eq_i16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel_eq_i16_vb
    
    result = v_f16_sel_eq_i16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f16_sel_eq_i16_vb(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f16_sel_eq_i16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f16_sel_eq_i16_vb(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_f16_sel_eq_i16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
     
  {
    volatile half128 __local *dptr = (half128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;
    
    half128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN3: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_f16_sel_eq_u16_b
    
    result = v_f16_sel_eq_u16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f16_sel_eq_u16_b(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f16_sel_eq_u16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f16_sel_eq_u16_b(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_f16_sel_eq_u16_b(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel_eq_u16_vb
    
    result = v_f16_sel_eq_u16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f16_sel_eq_u16_vb(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f16_sel_eq_u16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f16_sel_eq_u16_vb(x1, x2, x3, af16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_f16_sel_eq_u16_vb(x1, x2, x3, 1.5, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

 
  {
    volatile int64 __local *dptr = (int64 __local *)dest_ptr;
    volatile int64 __local *sptr = (int64 __local *)src_ptr;
    volatile int64 __local *rptr = (int64 __local *)res_ptr;
    
    int64 result = *dptr++;
    int64 x1 = *sptr++;
    int64 x2 = *sptr++;
    int64 x3 = *rptr++;
    int64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i32_sel_eq_i32_b
    
    result = v_i32_sel_eq_i32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i32_sel_eq_i32_b(x1, ai32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i32_sel_eq_i32_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i32_sel_eq_i32_b(x1, x2, x3, ai32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[SPRED]]     
    
    result = v_i32_sel_eq_i32_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i32_sel_eq_i32_vb
    
    result = v_i32_sel_eq_i32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i32_sel_eq_i32_vb(x1, ai32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i32_sel_eq_i32_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i32_sel_eq_i32_vb(x1, x2, x3, ai32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[VPRED]]     
    
    result = v_i32_sel_eq_i32_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile int64 __local *dptr = (int64 __local *)dest_ptr;
    volatile float64 __local *sptr = (float64 __local *)src_ptr;
    volatile int64 __local *rptr = (int64 __local *)res_ptr;
    
    int64 result = *dptr++;
    float64 x1 = *sptr++;
    float64 x2 = *sptr++;
    int64 x3 = *rptr++;
    int64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i32_sel_eq_f32_b
    
    result = v_i32_sel_eq_f32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i32_sel_eq_f32_b(x1, af32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i32_sel_eq_f32_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i32_sel_eq_f32_b(x1, x2, x3, ai32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[SPRED]]     
    
    result = v_i32_sel_eq_f32_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i32_sel_eq_f32_vb
    
    result = v_i32_sel_eq_f32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i32_sel_eq_f32_vb(x1, af32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i32_sel_eq_f32_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i32_sel_eq_f32_vb(x1, x2, x3, ai32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[VPRED]]     
    
    result = v_i32_sel_eq_f32_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile int64 __local *dptr = (int64 __local *)dest_ptr;
    volatile uint64 __local *sptr = (uint64 __local *)src_ptr;
    volatile int64 __local *rptr = (int64 __local *)res_ptr;
    
    int64 result = *dptr++;
    uint64 x1 = *sptr++;
    uint64 x2 = *sptr++;
    int64 x3 = *rptr++;
    int64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i32_sel_eq_u32_b
    
    result = v_i32_sel_eq_u32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i32_sel_eq_u32_b(x1, au32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i32_sel_eq_u32_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i32_sel_eq_u32_b(x1, x2, x3, ai32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[SPRED]]     
    
    result = v_i32_sel_eq_u32_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i32_sel_eq_u32_vb
    
    result = v_i32_sel_eq_u32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i32_sel_eq_u32_vb(x1, au32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i32_sel_eq_u32_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i32_sel_eq_u32_vb(x1, x2, x3, ai32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[VPRED]]     
    
    result = v_i32_sel_eq_u32_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile uint64 __local *dptr = (uint64 __local *)dest_ptr;
    volatile uint64 __local *sptr = (uint64 __local *)src_ptr;
    volatile uint64 __local *rptr = (uint64 __local *)res_ptr;
    
    uint64 result = *dptr++;
    uint64 x1 = *sptr++;
    uint64 x2 = *sptr++;
    uint64 x3 = *rptr++;
    uint64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u32_sel_eq_u32_b
    
    result = v_u32_sel_eq_u32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u32_sel_eq_u32_b(x1, au32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u32_sel_eq_u32_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u32_sel_eq_u32_b(x1, x2, x3, au32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[SPRED]]     
    
    result = v_u32_sel_eq_u32_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u32_sel_eq_u32_vb
    
    result = v_u32_sel_eq_u32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u32_sel_eq_u32_vb(x1, au32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u32_sel_eq_u32_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u32_sel_eq_u32_vb(x1, x2, x3, au32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[VPRED]]     
    
    result = v_u32_sel_eq_u32_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile uint64 __local *dptr = (uint64 __local *)dest_ptr;
    volatile float64 __local *sptr = (float64 __local *)src_ptr;
    volatile uint64 __local *rptr = (uint64 __local *)res_ptr;
    
    uint64 result = *dptr++;
    float64 x1 = *sptr++;
    float64 x2 = *sptr++;
    uint64 x3 = *rptr++;
    uint64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u32_sel_eq_f32_b
    
    result = v_u32_sel_eq_f32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u32_sel_eq_f32_b(x1, af32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u32_sel_eq_f32_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u32_sel_eq_f32_b(x1, x2, x3, au32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[SPRED]]     
    
    result = v_u32_sel_eq_f32_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u32_sel_eq_f32_vb
    
    result = v_u32_sel_eq_f32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u32_sel_eq_f32_vb(x1, af32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u32_sel_eq_f32_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u32_sel_eq_f32_vb(x1, x2, x3, au32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[VPRED]]     
    
    result = v_u32_sel_eq_f32_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.f32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile uint64 __local *dptr = (uint64 __local *)dest_ptr;
    volatile int64 __local *sptr = (int64 __local *)src_ptr;
    volatile uint64 __local *rptr = (uint64 __local *)res_ptr;
    
    uint64 result = *dptr++;
    int64 x1 = *sptr++;
    int64 x2 = *sptr++;
    uint64 x3 = *rptr++;
    uint64 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u32_sel_eq_i32_b
    
    result = v_u32_sel_eq_i32_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u32_sel_eq_i32_b(x1, ai32, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u32_sel_eq_i32_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u32_sel_eq_i32_b(x1, x2, x3, au32, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[SPRED]]     
    
    result = v_u32_sel_eq_i32_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u32_sel_eq_i32_vb
    
    result = v_u32_sel_eq_i32_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u32_sel_eq_i32_vb(x1, ai32, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u32_sel_eq_i32_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u32_sel_eq_i32_vb(x1, x2, x3, au32, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[VPRED]]     
    
    result = v_u32_sel_eq_i32_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i32 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile short128 __local *dptr = (short128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;
    
    short128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i16_sel_eq_i16_b
    
    result = v_i16_sel_eq_i16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i16_sel_eq_i16_b(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i16_sel_eq_i16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i16_sel_eq_i16_b(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_i16_sel_eq_i16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel_eq_i16_vb
    
    result = v_i16_sel_eq_i16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i16_sel_eq_i16_vb(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i16_sel_eq_i16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i16_sel_eq_i16_vb(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_i16_sel_eq_i16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

#if defined (__gaudi__) || defined(__goya2__)
     
  {
    volatile short128 __local *dptr = (short128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;
    
    short128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN2: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i16_sel_eq_bf16_b
    
    result = v_i16_sel_eq_bf16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i16_sel_eq_bf16_b(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i16_sel_eq_bf16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i16_sel_eq_bf16_b(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_i16_sel_eq_bf16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel_eq_bf16_vb
    
    result = v_i16_sel_eq_bf16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i16_sel_eq_bf16_vb(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i16_sel_eq_bf16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i16_sel_eq_bf16_vb(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_i16_sel_eq_bf16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
     
  {
    volatile short128 __local *dptr = (short128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;
    
    short128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN3: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i16_sel_eq_f16_b
    
    result = v_i16_sel_eq_f16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i16_sel_eq_f16_b(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i16_sel_eq_f16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i16_sel_eq_f16_b(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_i16_sel_eq_f16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel_eq_f16_vb
    
    result = v_i16_sel_eq_f16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i16_sel_eq_f16_vb(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i16_sel_eq_f16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i16_sel_eq_f16_vb(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_i16_sel_eq_f16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

 
  {
    volatile short128 __local *dptr = (short128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;
    
    short128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i16_sel_eq_u16_b
    
    result = v_i16_sel_eq_u16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i16_sel_eq_u16_b(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i16_sel_eq_u16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i16_sel_eq_u16_b(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     
    
    result = v_i16_sel_eq_u16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel_eq_u16_vb
    
    result = v_i16_sel_eq_u16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i16_sel_eq_u16_vb(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i16_sel_eq_u16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i16_sel_eq_u16_vb(x1, x2, x3, ai16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     
    
    result = v_i16_sel_eq_u16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile ushort128 __local *dptr = (ushort128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;
    
    ushort128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u16_sel_eq_u16_b
    
    result = v_u16_sel_eq_u16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u16_sel_eq_u16_b(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u16_sel_eq_u16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u16_sel_eq_u16_b(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_u16_sel_eq_u16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel_eq_u16_vb
    
    result = v_u16_sel_eq_u16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u16_sel_eq_u16_vb(x1, au16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u16_sel_eq_u16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u16_sel_eq_u16_vb(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_u16_sel_eq_u16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.u16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

#if defined (__gaudi__) || defined(__goya2__)
     
  {
    volatile ushort128 __local *dptr = (ushort128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;
    
    ushort128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN2: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u16_sel_eq_bf16_b
    
    result = v_u16_sel_eq_bf16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u16_sel_eq_bf16_b(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u16_sel_eq_bf16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u16_sel_eq_bf16_b(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_u16_sel_eq_bf16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel_eq_bf16_vb
    
    result = v_u16_sel_eq_bf16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u16_sel_eq_bf16_vb(x1, abf16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u16_sel_eq_bf16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u16_sel_eq_bf16_vb(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_u16_sel_eq_bf16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel_eq.bf16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
     
  {
    volatile ushort128 __local *dptr = (ushort128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;
    
    ushort128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;
    
    *dptr++ = result;
    // GEN3: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u16_sel_eq_f16_b
    
    result = v_u16_sel_eq_f16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u16_sel_eq_f16_b(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u16_sel_eq_f16_b(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u16_sel_eq_f16_b(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_u16_sel_eq_f16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel_eq_f16_vb
    
    result = v_u16_sel_eq_f16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u16_sel_eq_f16_vb(x1, af16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u16_sel_eq_f16_vb(x1, 1.5, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u16_sel_eq_f16_vb(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_u16_sel_eq_f16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel_eq.f16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

 
  {
    volatile ushort128 __local *dptr = (ushort128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;
    
    ushort128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u16_sel_eq_i16_b
    
    result = v_u16_sel_eq_i16_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u16_sel_eq_i16_b(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u16_sel_eq_i16_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u16_sel_eq_i16_b(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     
    
    result = v_u16_sel_eq_i16_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel_eq_i16_vb
    
    result = v_u16_sel_eq_i16_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u16_sel_eq_i16_vb(x1, ai16, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u16_sel_eq_i16_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u16_sel_eq_i16_vb(x1, x2, x3, au16, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     
    
    result = v_u16_sel_eq_i16_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel_eq.i16 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile char256 __local *dptr = (char256 __local *)dest_ptr;
    volatile char256 __local *sptr = (char256 __local *)src_ptr;
    volatile char256 __local *rptr = (char256 __local *)res_ptr;
    
    char256 result = *dptr++;
    char256 x1 = *sptr++;
    char256 x2 = *sptr++;
    char256 x3 = *rptr++;
    char256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i8_sel_eq_i8_b
    
    result = v_i8_sel_eq_i8_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i8_sel_eq_i8_b(x1, ai8, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i8_sel_eq_i8_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i8_sel_eq_i8_b(x1, x2, x3, ai8, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[SPRED]]     
    
    result = v_i8_sel_eq_i8_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i8_sel_eq_i8_vb
    
    result = v_i8_sel_eq_i8_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i8_sel_eq_i8_vb(x1, ai8, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i8_sel_eq_i8_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i8_sel_eq_i8_vb(x1, x2, x3, ai8, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[VPRED]]     
    
    result = v_i8_sel_eq_i8_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile char256 __local *dptr = (char256 __local *)dest_ptr;
    volatile uchar256 __local *sptr = (uchar256 __local *)src_ptr;
    volatile char256 __local *rptr = (char256 __local *)res_ptr;
    
    char256 result = *dptr++;
    uchar256 x1 = *sptr++;
    uchar256 x2 = *sptr++;
    char256 x3 = *rptr++;
    char256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_i8_sel_eq_u8_b
    
    result = v_i8_sel_eq_u8_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_i8_sel_eq_u8_b(x1, au8, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_i8_sel_eq_u8_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_i8_sel_eq_u8_b(x1, x2, x3, ai8, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[SPRED]]     
    
    result = v_i8_sel_eq_u8_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i8_sel_eq_u8_vb
    
    result = v_i8_sel_eq_u8_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_i8_sel_eq_u8_vb(x1, au8, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_i8_sel_eq_u8_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_i8_sel_eq_u8_vb(x1, x2, x3, ai8, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[VPRED]]     
    
    result = v_i8_sel_eq_u8_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile uchar256 __local *dptr = (uchar256 __local *)dest_ptr;
    volatile uchar256 __local *sptr = (uchar256 __local *)src_ptr;
    volatile uchar256 __local *rptr = (uchar256 __local *)res_ptr;
    
    uchar256 result = *dptr++;
    uchar256 x1 = *sptr++;
    uchar256 x2 = *sptr++;
    uchar256 x3 = *rptr++;
    uchar256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u8_sel_eq_u8_b
    
    result = v_u8_sel_eq_u8_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u8_sel_eq_u8_b(x1, au8, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u8_sel_eq_u8_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u8_sel_eq_u8_b(x1, x2, x3, au8, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[SPRED]]     
    
    result = v_u8_sel_eq_u8_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u8_sel_eq_u8_vb
    
    result = v_u8_sel_eq_u8_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u8_sel_eq_u8_vb(x1, au8, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u8_sel_eq_u8_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u8_sel_eq_u8_vb(x1, x2, x3, au8, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[VPRED]]     
    
    result = v_u8_sel_eq_u8_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.u8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

 
  {
    volatile uchar256 __local *dptr = (uchar256 __local *)dest_ptr;
    volatile char256 __local *sptr = (char256 __local *)src_ptr;
    volatile uchar256 __local *rptr = (uchar256 __local *)res_ptr;
    
    uchar256 result = *dptr++;
    char256 x1 = *sptr++;
    char256 x2 = *sptr++;
    uchar256 x3 = *rptr++;
    uchar256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]    


    //------ v_u8_sel_eq_i8_b
    
    result = v_u8_sel_eq_i8_b(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_u8_sel_eq_i8_b(x1, ai8, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_u8_sel_eq_i8_b(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_u8_sel_eq_i8_b(x1, x2, x3, au8, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[SPRED]]     
    
    result = v_u8_sel_eq_i8_b(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u8_sel_eq_i8_vb
    
    result = v_u8_sel_eq_i8_vb(x1, x2, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_u8_sel_eq_i8_vb(x1, ai8, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_u8_sel_eq_i8_vb(x1, 123, x3, x4, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_u8_sel_eq_i8_vb(x1, x2, x3, au8, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[VPRED]]     
    
    result = v_u8_sel_eq_i8_vb(x1, x2, x3, 123, SW_MASK_EQ_ZERO, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_eq.i8 mask_eq_zero [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 
}
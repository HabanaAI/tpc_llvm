// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck %s
  void main(int dest, int src1, int src2, int pred_val, int vect_pred,
          float af32, int ai32, unsigned au32, 
          char ai8, unsigned char au8) {
  int64 __local *dest_ptr = (int64 __local *)dest;
  int64 __local *src_ptr = (int64 __local *)src1;          
  int64 __local *res_ptr = (int64 __local *)src2;
  _Bool spred = src1 < src2; 
  bool256 __local *vpred_ptr = (bool256 __local *)vect_pred;
  bool256 vpred = *vpred_ptr;

  minifloat af8= (minifloat)au8;
  minihalf ah8= (minihalf)au8;
  // CHECK-DAG: cmp_less.i32 [[SPRED:%SP[0-9]+]], %S1, %S2
  // CHECK-DAG: ld_l_v [[VPRED:%VP[0-9]+]]

  if (dest > 13) {
    volatile minifloat256 __local *dptr = (minifloat256 __local *)dest_ptr;
    volatile minifloat256 __local *sptr = (minifloat256 __local *)src_ptr;
    volatile minifloat256 __local *rptr = (minifloat256 __local *)res_ptr;
    
    minifloat256 result = *dptr++;
    minifloat256 x1 = *sptr++;
    minifloat256 x2 = *sptr++;
    minifloat256 x3 = *rptr++;
    minifloat256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]


    //------ v_f8_sel_leq_f8_b
    
    result = v_f8_sel_leq_f8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_f8_b(x1, af8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f8_sel_leq_f8_b(x1, (minifloat)1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, 0x3c, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f8_sel_leq_f8_b(x1, x2, x3, af8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_f8_b(x1, x2, x3, (minifloat)1.5, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[SPRED]]

    //------ v_f8_sel_leq_f8_vb
    
    result = v_f8_sel_leq_f8_vb(x1, x2, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_f8_vb(x1, af8, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f8_sel_leq_f8_vb(x1, (minifloat)1.5, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, 0x3c, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f8_sel_leq_f8_vb(x1, x2, x3, af8, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_f8_vb(x1, x2, x3, (minifloat)1.5, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
  if (dest > 14) {
    volatile minifloat256 __local *dptr = (minifloat256 __local *)dest_ptr;
    volatile minihalf256 __local *sptr = (minihalf256 __local *)src_ptr;
    volatile minifloat256 __local *rptr = (minifloat256 __local *)res_ptr;
    
    minifloat256 result = *dptr++;
    minihalf256 x1 = *sptr++;
    minihalf256 x2 = *sptr++;
    minifloat256 x3 = *rptr++;
    minifloat256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]


    //------ v_f8_sel_leq_h8_b
    
    result = v_f8_sel_leq_h8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_h8_b(x1, ah8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f8_sel_leq_h8_b(x1, (minihalf)1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, 0x3e, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f8_sel_leq_h8_b(x1, x2, x3, af8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_h8_b(x1, x2, x3, (minifloat)1.5, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[SPRED]]

    //------ v_f8_sel_leq_h8_vb
    
    result = v_f8_sel_leq_h8_vb(x1, x2, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_h8_vb(x1, ah8, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f8_sel_leq_h8_vb(x1, (minihalf)1.5, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, 0x3e, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f8_sel_leq_h8_vb(x1, x2, x3, af8, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_h8_vb(x1, x2, x3, (minifloat)1.5, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_152  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
  if (dest > 15) {
    volatile minifloat256 __local *dptr = (minifloat256 __local *)dest_ptr;
    volatile char256 __local *sptr = (char256 __local *)src_ptr;
    volatile minifloat256 __local *rptr = (minifloat256 __local *)res_ptr;
    
    minifloat256 result = *dptr++;
    char256 x1 = *sptr++;
    char256 x2 = *sptr++;
    minifloat256 x3 = *rptr++;
    minifloat256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]


    //------ v_f8_sel_leq_i8_b
    
    result = v_f8_sel_leq_i8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_i8_b(x1, ai8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f8_sel_leq_i8_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f8_sel_leq_i8_b(x1, x2, x3, af8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_i8_b(x1, x2, x3, (minifloat)1.5, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[SPRED]]

    //------ v_f8_sel_leq_i8_vb
    
    result = v_f8_sel_leq_i8_vb(x1, x2, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_i8_vb(x1, ai8, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f8_sel_leq_i8_vb(x1, 123, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f8_sel_leq_i8_vb(x1, x2, x3, af8, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_i8_vb(x1, x2, x3, (minifloat)1.5, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.i8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
  if (dest > 16) {
    volatile minifloat256 __local *dptr = (minifloat256 __local *)dest_ptr;
    volatile uchar256 __local *sptr = (uchar256 __local *)src_ptr;
    volatile minifloat256 __local *rptr = (minifloat256 __local *)res_ptr;
    
    minifloat256 result = *dptr++;
    uchar256 x1 = *sptr++;
    uchar256 x2 = *sptr++;
    minifloat256 x3 = *rptr++;
    minifloat256 x4 = *rptr++;
    
    *dptr++ = result;
    // CHECK: st_l_v {{.*}}, [[DEST:%V[0-9]+]]

    //------ v_f8_sel_leq_u8_b
    
    result = v_f8_sel_leq_u8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_u8_b(x1, au8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    
    
    result = v_f8_sel_leq_u8_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   
    
    result = v_f8_sel_leq_u8_b(x1, x2, x3, af8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[SPRED]]     
    
    result = v_f8_sel_leq_u8_b(x1, x2, x3, (minifloat)1.5, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[SPRED]]

    //------ v_f8_sel_leq_u8_vb
    
    result = v_f8_sel_leq_u8_vb(x1, x2, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_u8_vb(x1, au8, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    
    
    result = v_f8_sel_leq_u8_vb(x1, 123, x3, x4, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   
    
    result = v_f8_sel_leq_u8_vb(x1, x2, x3, af8, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, [[VPRED]]     
    
    result = v_f8_sel_leq_u8_vb(x1, x2, x3, (minifloat)1.5, 0, result, (vpred), 0);
    *dptr++ = result;
    // CHECK: sel_leq.u8  [[DEST]], %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, [[VPRED]]

    dest_ptr = (int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
     
 }

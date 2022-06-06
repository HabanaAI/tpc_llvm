// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck %s

  void main(int dest, int src1, int src2, int pred_val, int vect_pred) {
  	int64 __local *dest_ptr = (int64 __local *)dest;
  	int64 __local *src_ptr  = (int64 __local *)src1;          
  	int64 __local *res_ptr  = (int64 __local *)src2;
  	_Bool spred = src1 < src2; 
  	bool256 __local *vpred_ptr = (bool256 __local *)vect_pred;
  	bool256 vpred = *vpred_ptr;

    volatile minifloat256 __local *dptr = (minifloat256 __local *)dest_ptr;
    volatile minifloat256 __local *sptr = (minifloat256 __local *)src_ptr;
    volatile minifloat256 __local *rptr = (minifloat256 __local *)res_ptr;
    
    minifloat256 result = *dptr++;
    minifloat256 x1 = *sptr++;
    minifloat256 x2 = *sptr++;
    minifloat256 x3 = *rptr++;
    minifloat256 x4 = *rptr++;
    
    *dptr++ = result;

    //------ v_f8_sel_geq_f8_b
    result = v_f8_sel_geq_f8_b(x1, x2, x3, x4, SW_SUP_NAN, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_geq.f8_143  sup_nan %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}    

    //------ v_f8_sel_geq_f8_vb
    result = v_f8_sel_geq_f8_vb(x1, x2, x3, x4, SW_SUP_NAN, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_geq.f8_143  sup_nan %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}    

    //------ v_f8_sel_grt_f8_b
    result = v_f8_sel_grt_f8_b(x1, x2, x3, x4, SW_SUP_NAN, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_grt.f8_143  sup_nan %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}    

    //------ v_f8_sel_geq_f8_vb
    result = v_f8_sel_grt_f8_vb(x1, x2, x3, x4, SW_SUP_NAN, result, vpred, 0);
    *dptr++ = result;

    // CHECK: sel_grt.f8_143  sup_nan %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}    

    //------ v_f8_sel_leq_f8_b
    result = v_f8_sel_leq_f8_b(x1, x2, x3, x4, SW_SUP_NAN, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel_leq.f8_143  sup_nan %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}    

    //------ v_f8_sel_less_f8_vb
    result = v_f8_sel_less_f8_vb(x1, x2, x3, x4, SW_SUP_NAN, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel_less.f8_143  sup_nan %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}    
 }

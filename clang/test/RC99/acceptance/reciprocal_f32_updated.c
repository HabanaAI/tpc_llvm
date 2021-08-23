// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

// GAUDI-250

// See also: GAUDI-164 (test needs to be corrected after the fix)

#define UNIT_VAL            0x3f800000
#define SIGN_MASK           0x80000000
#define EXPONENT_MASK       0x7f800000
#define SIGNIFICAND_MASK    0x007fffff
#define BIASED_EXP_0_SHIFTED 0xc0800000
#define EXPONENT_MASK_F32   0x7f800000
#define NAN_FP32            0x7fffffff
#define PLUS_INF_FP32       0x7f800000
#define MINUS_INF_FP32      0xff800000
#define FLT_MIN             0x800000
#define FLT_MAX             0x7f7fffff

#define false  0
#define true   1

float64 reciprocal_f32(float64 input)
{
    bool256 all_true_pred = 0xff;
    //FORM_FP_NUMBER.F32 V1, $UNIT_VAL$, V10, V10, FORCE_SIGN0
    unsigned int unit_val = UNIT_VAL;
//    float64  v1 = v_f32_form_fp_num_s_v_v_vb(*((float*)&unit_val),input,input,false,true,false,false,false,all_true_pred,0 );
    float64  v1 =0;
    v1 = v_f32_form_fp_num_s_v_v_vb(*((float*)&unit_val),input,input,v1, 0x8,all_true_pred,0 );
    //GET_LUT_ENTRY_AND_INTERVAL_START.F32 V2, V3, V1, u32_x110010 - ask sergei- this does not make sense.
//    float64_pair_t pairv2v3 =  v_f32_get_lut_entry_and_interval_start_v_b(v1,0/*significant shift*/,/*e_func_variant_default*/0,1,0);
    uint64_float64_pair_t pairv2v3 ;
    pairv2v3 = v_f32_get_lut_entry_and_interval_start_v_b(v1,pairv2v3,0/*significant shift*/,/*e_func_variant_default*/0,1,0);
    //GET_LUT_ENTRY_AND_INTERVAL_START.F32 V2, V3, V1, 16, 0
    pairv2v3 = v_f32_get_lut_entry_and_interval_start_v_b(v1,pairv2v3,16,0,1,0);
    //LOOKUP_C1C2 V4, V2, 17, F32 - also ask sergey.why 17?
//    float64_pair_t pairv4v5 = v_f32_lookup_c1c2_v_b(pairv2v3.v1, 0 /*e_lookup_fp32*/,0/* e_reciprocal*/ ,1,0 );
    float64_pair_t pairv4v5;
    pairv4v5 = v_f32_lookup_c1c2_v_b(pairv2v3.v1, pairv4v5, 0 /*e_lookup_fp32*/,0/* e_reciprocal*/ ,1,0 );
    //SUB.F32 V3, V1, V3
    pairv2v3.v2 = v1 - pairv2v3.v2;
    // LOOKUP_C0 V5, V2, 17, F32
//    float64 v5 = v_f32_lookup_c0_v_b(pairv2v3.v1,0 /*e_lookup_fp32*/,0 /* e_reciprocal */,1,0 );
    float64 v5;
    v5 = v_f32_lookup_c0_v_b(pairv2v3.v1, v5,0 /*e_lookup_fp32*/,0 /* e_reciprocal */,1,0 );
    // MAC.F32 V4, V3, V5

    // GAUDI-164
    float64 v2_temp = pairv2v3.v2;
    v2_temp = v_f32_mac_v_v_b(pairv2v3.v2, v5, v2_temp, false,1,0);
    pairv2v3.v2 = v2_temp;
    // Workaround for GAUDI-164 can be the following:
    // float64 v7 = pairv2v3.v2;
    // v7 = v_f32_mac_v_v_b(pairv2v3.v2, v5, v7, false,1,0);
    // pairv2v3.v2 = v7;

    //MAC.F32 V5, V3, V4
//    v5 = v_f32_mac_v_v_b(pairv2v3.v2 , pairv4v5.v1, v5,false,1,0);
    v5 = v_f32_mac_v_v_b(pairv2v3.v2 , pairv4v5.v1, v5,false,1,0);
    //AND.U32 V1, V10, $EXPONENT_MASK$
    uint64 a1 = EXPONENT_MASK & *((uint64*)&input);
    //AND.U32 V2, V5, $EXPONENT_MASK$
    uint64 a2 = *((uint64*)&(v5)) & EXPONENT_MASK;
    //SUB.I32 V2, V2, V1 - Sergei - question do we must do signed subtraction?
    a2 = a2 - a1;
    //CMP_LEQ.I32 VP1, V2, BIASED_EXP_0_SHIFTED
    bool256 vp1;
    vp1 = bv_u32_cmp_leq_v_v_vb(a2,BIASED_EXP_0_SHIFTED, vp1, all_true_pred,0);
    //FORM_FP_NUMBER.F32 V2, V2, V10, V5, EXP_ADD_BIAS
//    float64 result = v_f32_form_fp_num_v_v_v_b(*((float64*)&(a2)),input,v5,true /*exp_bias*/,false,false,false,false,1,0);
    float64 result;
    result = v_f32_form_fp_num_v_v_v_b(*((float64*)&(a2)),input,v5,result,0x10, /*exp_bias,false,false,false,false, */1,0);
    //MOV.F32 V2, f32_0, VP1 - MOV is implemented with ADD (X + 0 )
    result = v_f32_add_v_s_vb(result,0,result,vp1,0);
    // ABS.F32 V11, V10
//    float64 v11 = v_f32_abs_v_b(result,1,0);
    uint64 v111 = *((uint64*)&(result)) & 0x7fffffff;
    float64 v11 = *((float64*)&v111);
    //AND.U32 V0, V10, $SIGN_MASK$
    uint64 v0 =  *((uint64*)&input) & SIGN_MASK;
    // OR.U32 V1, V0, $PLUS_INF_FP32$
    uint64 v1_uint = v0 | PLUS_INF_FP32;
    //SEL_LESS.F32 V2, V11, $FLT_MIN$, V1, V2
    v1 = *((float64*) &v1_uint);
    result = v_f32_f32_sel_less_v_v_v_v_vb(v11,FLT_MIN,v1,result,result,all_true_pred,1);
    //SEL_EQ.U32 V2, V11, $PLUS_INF_FP32$, V0, V2
    uint64 result2;
    result2 = v_u32_u32_sel_eq_v_v_v_v_vb(*((uint64*)&(v11)),PLUS_INF_FP32,v0,*((uint64*)&(result)),result2,all_true_pred,1);
    //cast back to float
    result = *((float64*)&(result2));
    //SEL_EQ.F32 V2, V10, V10, V2, $NAN_FP32$
    uint64 Nan = 0x7fffffff;
    result =  v_f32_f32_sel_eq_v_v_v_v_vb(input,input,result, *((float64*)&(Nan)),result,all_true_pred,1);
    
    //todo: add 
    return result;
}


// 
void main(tensor input, tensor output)
{
    int5 addr=  0; 
    // spatial for loops
    float64 tmp;
    tmp = v_f32_ld_tnsr_i_b(addr,input,tmp,1,0);
    tmp = reciprocal_f32(tmp);
    f32_st_tnsr_i_v_b(addr,output,tmp,1,0);
}

// CHECK: main
// CHECK: halt

// SPDX-License-Identifier: Apache-2.0
// C2-C6 FP32 control: share four weight columns across three residual rows.
// Partition K so the six-row workload exposes 240 independent work points.
// Arithmetic stays FP32; partial sums are consumed by the fixed reducer.
#pragma clang fp contract(off)
void main(tensor activation, tensor weight, tensor partials) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int tokens=get_dim_size(activation,1);
    for(int tile=begin[2];tile<end[2];++tile)
    for(int rows=begin[1];rows<end[1];++rows)
    for(int cols=begin[0];cols<end[0];++cols) {
        const int r=rows*3,c=cols*4;
        float64 a00=0,a01=0,a02=0,a03=0;
        float64 a10=0,a11=0,a12=0,a13=0;
        float64 a20=0,a21=0,a22=0,a23=0;
        float64 n0=0,n1=0,n2=0;
        for(int k=tile*1024;k<(tile+1)*1024;k+=128) {
            const float128 x0=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(
                (int5){k,r},activation,0,(bfloat128)0,r<tokens),SW_LINEAR);
            const float128 x1=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(
                (int5){k,r+1},activation,0,(bfloat128)0,r+1<tokens),SW_LINEAR);
            const float128 x2=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(
                (int5){k,r+2},activation,0,(bfloat128)0,r+2<tokens),SW_LINEAR);
            #define DOT(COL,A,B,C) { \
                const float64 w0=v_f32_ld_tnsr_b((int5){k,c+COL},weight); \
                const float64 w1=v_f32_ld_tnsr_b((int5){k+64,c+COL},weight); \
                A=v_f32_mac_b(x0.v1,w0,A);A=v_f32_mac_b(x0.v2,w1,A); \
                B=v_f32_mac_b(x1.v1,w0,B);B=v_f32_mac_b(x1.v2,w1,B); \
                C=v_f32_mac_b(x2.v1,w0,C);C=v_f32_mac_b(x2.v2,w1,C); \
            }
            DOT(0,a00,a10,a20);DOT(1,a01,a11,a21);
            DOT(2,a02,a12,a22);DOT(3,a03,a13,a23);
            #undef DOT
            if(cols==0) {
                n0+=(x0.v1*x0.v1)+(x0.v2*x0.v2);
                n1+=(x1.v1*x1.v1)+(x1.v2*x1.v2);
                n2+=(x2.v1*x2.v1)+(x2.v2*x2.v2);
            }
        }
        #define STORE(ROW,COL,VALUE) \
            if(r+ROW<tokens)v_f32_st_tnsr_partial((int5){c+COL,tile,r+ROW},partials, \
                v_f32_reduce_add(VALUE),0,0)
        STORE(0,0,a00);STORE(0,1,a01);STORE(0,2,a02);STORE(0,3,a03);
        STORE(1,0,a10);STORE(1,1,a11);STORE(1,2,a12);STORE(1,3,a13);
        STORE(2,0,a20);STORE(2,1,a21);STORE(2,2,a22);STORE(2,3,a23);
        #undef STORE
        if(cols==0) {
            if(r<tokens)v_f32_st_tnsr_partial((int5){24,tile,r},partials,v_f32_reduce_add(n0),0,0);
            if(r+1<tokens)v_f32_st_tnsr_partial((int5){24,tile,r+1},partials,v_f32_reduce_add(n1),0,0);
            if(r+2<tokens)v_f32_st_tnsr_partial((int5){24,tile,r+2},partials,v_f32_reduce_add(n2),0,0);
        }
    }
}

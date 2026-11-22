// SPDX-License-Identifier: Apache-2.0
// Each work point owns two independent heads sharing selected KV loads.
static inline float256 exp_four(float64 a, float64 b, float64 c, float64 d,
                                uint64 lane)
{
    float64 arguments = v_f32_mov_vb(b, 0, a, v_u32_cmp_eq_b(lane, 1));
    arguments = v_f32_mov_vb(c, 0, arguments, v_u32_cmp_eq_b(lane, 2));
    arguments = v_f32_mov_vb(d, 0, arguments, v_u32_cmp_eq_b(lane, 3));
    const float64 values = v_exp_cephes_f32(arguments);
    float256 result;
    result.v1 = v_f32_shuffle_b(values, (uchar256)0x80, 0, values);
    result.v2 = v_f32_shuffle_b(values, (uchar256)0x81, 0, values);
    result.v3 = v_f32_shuffle_b(values, (uchar256)0x82, 0, values);
    result.v4 = v_f32_shuffle_b(values, (uchar256)0x83, 0, values);
    return result;
}

void main(tensor q, tensor kv, tensor indices, tensor sink, tensor scale,
          tensor lengths, tensor output, tensor maxima, tensor lse)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int sequence_length = get_dim_size(kv, 1);
    const int topk_width = get_dim_size(indices, 0);
    const float scale_value = s_f32_ld_g(gen_addr((int5){0}, scale));
    const uint64 lanes = read_lane_id_4b_b() & 3;
    for (int batch = start[2]; batch < end[2]; ++batch) {
        int width = s_i32_ld_g(gen_addr((int5){batch,0,0,0,0}, lengths));
        if (width < 0) width = 0;
        if (width > topk_width) width = topk_width;
        for (int pair = start[1]; pair < end[1]; ++pair) {
            bfloat128 queries[2][4];
            float128 accum[2][4] = {0};
            float64 maximum[2] = {-3.402823466e+38f, -3.402823466e+38f};
            float64 sum[2] = {0};
            #pragma unroll (2)
            for (int h=0; h<2; ++h) {
                #pragma unroll (4)
                for (int c=0; c<4; ++c)
                    queries[h][c] = v_bf16_ld_tnsr_b((int5){c*128,pair*2+h,batch,0,0},q);
            }
            for (int position=0; position<width; ++position) {
                const int row=s_i32_ld_g(gen_addr((int5){position,batch,0,0,0},indices));
                if (row < 0 || row >= sequence_length) continue;
                bfloat128 values[4];
                float128 dots[2] = {0};
                #pragma unroll (4)
                for (int c=0; c<4; ++c) {
                    values[c]=v_bf16_ld_tnsr_b((int5){c*128,row,0,0,0},kv);
                    #pragma unroll (2)
                    for (int h=0; h<2; ++h)
                        dots[h]=v_bf16_mac_acc32_b(queries[h][c],values[c],dots[h],(e_no_negation)<<1);
                }
                float64 score[2], next[2];
                #pragma unroll (2)
                for (int h=0; h<2; ++h) {
                    float64 reduced=v_f32_reduce_add(dots[h].v1+dots[h].v2);
                    score[h]=v_f32_shuffle_b(reduced,(uchar256)0x80,0,reduced)*scale_value;
                    next[h]=v_f32_max_b(maximum[h],score[h]);
                }
                const float256 exponent=exp_four(maximum[0]-next[0],score[0]-next[0],
                                                  maximum[1]-next[1],score[1]-next[1],lanes);
                const float64 prior[2]={exponent.v1,exponent.v3};
                const float64 weight[2]={exponent.v2,exponent.v4};
                #pragma unroll (2)
                for (int h=0; h<2; ++h) {
                    sum[h]=sum[h]*prior[h]+weight[h];
                    maximum[h]=next[h];
                }
                #pragma unroll (4)
                for (int c=0; c<4; ++c) {
                    const float128 value=v_convert_bf16_to_f32_all_b(values[c]);
                    #pragma unroll (2)
                    for (int h=0; h<2; ++h) {
                        accum[h][c].v1=v_f32_mac_b(value.v1,weight[h],accum[h][c].v1*prior[h]);
                        accum[h][c].v2=v_f32_mac_b(value.v2,weight[h],accum[h][c].v2*prior[h]);
                    }
                }
            }
            float64 final[2], sink_score[2];
            #pragma unroll (2)
            for (int h=0; h<2; ++h) {
                sink_score[h]=s_f32_ld_g(gen_addr((int5){pair*2+h,0,0,0,0},sink));
                final[h]=v_f32_max_b(maximum[h],sink_score[h]);
            }
            const float256 exponent=exp_four(maximum[0]-final[0],sink_score[0]-final[0],
                                              maximum[1]-final[1],sink_score[1]-final[1],lanes);
            const float64 prior[2]={exponent.v1,exponent.v3};
            const float64 sink_weight[2]={exponent.v2,exponent.v4};
            #pragma unroll (2)
            for (int h=0; h<2; ++h) {
                sum[h]=sum[h]*prior[h]+sink_weight[h];
                const float64 inverse=v_reciprocal_f32(sum[h]);
                const float64 logsum=final[h]+v_log_f32(sum[h]);
                #pragma unroll (4)
                for (int c=0; c<4; ++c) {
                    accum[h][c].v1*=prior[h]*inverse;
                    accum[h][c].v2*=prior[h]*inverse;
                    const bfloat128 value=v_convert_f32_to_bf16_all_b(accum[h][c]);
                    v_bf16_st_tnsr((int5){c*128,pair*2+h,batch,0,0},output,value);
                }
                v_f32_st_tnsr_partial((int5){pair*2+h,batch,0,0,0},maxima,final[h],0,0);
                v_f32_st_tnsr_partial((int5){pair*2+h,batch,0,0,0},lse,logsum,0,0);
            }
        }
    }
}

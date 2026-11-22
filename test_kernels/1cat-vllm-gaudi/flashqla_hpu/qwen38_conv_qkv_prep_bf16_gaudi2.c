// Qwen3.8-27B GDN width-4 causal convolution and Q/K/V preparation.
//
// The unfused graph materializes a [tokens, 10240] BF16 convolution output
// before Q/K normalization.  This kernel consumes the projection output and
// the three-token convolution history directly, then writes only the layouts
// needed by the GDN core.

#ifdef QWEN38_CONV_QKV_NO_SILU
#define APPLY_SILU(VALUE) (VALUE)
#elif defined(QWEN38_CONV_QKV_NO_SAT_SIGMOID)
#define APPLY_SILU(VALUE)                                                \
    v_bf16_mul_b((VALUE), no_saturation_sigmoid_bf16((VALUE)))
#elif defined(QWEN38_CONV_QKV_EXP_NOLUT_FAST_RECIP)
#define APPLY_SILU(VALUE)                                                \
    v_bf16_mul_b(                                                        \
        (VALUE),                                                         \
        custom_fast_reciprocal_bf16(                                    \
            (bf16)1.0f + exp_fast_nolut(-(VALUE))))
#elif defined(QWEN38_CONV_QKV_EXP_NOLUT_RECIP)
#define APPLY_SILU(VALUE)                                                \
    v_bf16_mul_b(                                                        \
        (VALUE),                                                         \
        reciprocal_bf16(                                                \
            (bf16)1.0f + exp_fast_nolut(-(VALUE))))
#elif defined(QWEN38_CONV_QKV_EXP4_FAST_RECIP)
#define APPLY_SILU(VALUE)                                                \
    v_bf16_mul_b(                                                        \
        (VALUE),                                                         \
        custom_fast_reciprocal_bf16(                                    \
            (bf16)1.0f + exp_approximated_4th_order_bf16(-(VALUE))))
#elif defined(QWEN38_CONV_QKV_EXP4_RECIP)
#define APPLY_SILU(VALUE)                                                \
    v_bf16_mul_b(                                                        \
        (VALUE),                                                         \
        reciprocal_bf16(                                                \
            (bf16)1.0f + exp_approximated_4th_order_bf16(-(VALUE))))
#else
#define APPLY_SILU(VALUE) v_bf16_mul_b((VALUE), sigmoid_bf16((VALUE)))
#endif

#define DECLARE_CONV_WEIGHTS(PREFIX, CHANNEL_OFFSET)                      \
    int5 PREFIX##_weight_coords = {CHANNEL_OFFSET, 0, 0, 0, 0};          \
    int5 PREFIX##_bias_coords = {CHANNEL_OFFSET, 0, 0, 0, 0};            \
    const bfloat128 PREFIX##_weight0 =                                    \
        v_bf16_ld_tnsr_b(PREFIX##_weight_coords, conv_weight);            \
    PREFIX##_weight_coords[1] = 1;                                        \
    const bfloat128 PREFIX##_weight1 =                                    \
        v_bf16_ld_tnsr_b(PREFIX##_weight_coords, conv_weight);            \
    PREFIX##_weight_coords[1] = 2;                                        \
    const bfloat128 PREFIX##_weight2 =                                    \
        v_bf16_ld_tnsr_b(PREFIX##_weight_coords, conv_weight);            \
    PREFIX##_weight_coords[1] = 3;                                        \
    const bfloat128 PREFIX##_weight3 =                                    \
        v_bf16_ld_tnsr_b(PREFIX##_weight_coords, conv_weight);            \
    const bfloat128 PREFIX##_bias =                                       \
        v_bf16_ld_tnsr_b(PREFIX##_bias_coords, conv_bias)

#define LOAD_CONV_SILU(PREFIX, CHANNEL_OFFSET, RESULT)                    \
    do                                                                    \
    {                                                                     \
        int5 input_coords = {CHANNEL_OFFSET, 0, 0, 0, 0};                \
        int5 state_coords = {CHANNEL_OFFSET, 0, 0, 0, 0};                \
        bfloat128 source;                                                 \
        bfloat128 accumulator = (bf16)0.0f;                               \
                                                                          \
        if (token >= 3)                                                    \
        {                                                                 \
            input_coords[1] = token - 3;                                  \
            source = v_bf16_ld_tnsr_b(input_coords, packed_qkv);          \
        }                                                                 \
        else                                                              \
        {                                                                 \
            state_coords[1] = token;                                      \
            source = v_bf16_ld_tnsr_b(state_coords, initial_state);       \
        }                                                                 \
        accumulator = v_bf16_mac_b(                                       \
            source, PREFIX##_weight0, accumulator);                       \
                                                                          \
        if (token >= 2)                                                    \
        {                                                                 \
            input_coords[1] = token - 2;                                  \
            source = v_bf16_ld_tnsr_b(input_coords, packed_qkv);          \
        }                                                                 \
        else                                                              \
        {                                                                 \
            state_coords[1] = token + 1;                                  \
            source = v_bf16_ld_tnsr_b(state_coords, initial_state);       \
        }                                                                 \
        accumulator = v_bf16_mac_b(                                       \
            source, PREFIX##_weight1, accumulator);                       \
                                                                          \
        if (token >= 1)                                                    \
        {                                                                 \
            input_coords[1] = token - 1;                                  \
            source = v_bf16_ld_tnsr_b(input_coords, packed_qkv);          \
        }                                                                 \
        else                                                              \
        {                                                                 \
            state_coords[1] = token + 2;                                  \
            source = v_bf16_ld_tnsr_b(state_coords, initial_state);       \
        }                                                                 \
        accumulator = v_bf16_mac_b(                                       \
            source, PREFIX##_weight2, accumulator);                       \
                                                                          \
        input_coords[1] = token;                                          \
        source = v_bf16_ld_tnsr_b(input_coords, packed_qkv);              \
        accumulator = v_bf16_mac_b(                                       \
            source, PREFIX##_weight3, accumulator);                       \
        accumulator = v_bf16_add_b(accumulator, PREFIX##_bias);           \
        RESULT = APPLY_SILU(accumulator);                                  \
    } while (0)

void main(
    tensor packed_qkv,
    tensor initial_state,
    tensor conv_weight,
    tensor conv_bias,
    tensor q_out,
    tensor k_out,
    tensor v_out)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const uchar256 broadcast_lane_zero = 0x80;
    const int qk_heads = 16;
    const int head_dim = 128;
    const int key_offset = qk_heads * head_dim;
    const int value_offset = 2 * key_offset;

    int5 output_coords = {0, 0, 0, 0, 0};

    for (int head = start[1]; head < end[1]; ++head)
    {
        DECLARE_CONV_WEIGHTS(q, head * head_dim);
        DECLARE_CONV_WEIGHTS(k, key_offset + head * head_dim);
        DECLARE_CONV_WEIGHTS(v, value_offset + head * head_dim);

        for (int token = start[0]; token < end[0]; ++token)
        {
            output_coords[2] = token;

            if (head < qk_heads)
            {
                bfloat128 q_bf16;
                bfloat128 k_bf16;
                LOAD_CONV_SILU(q, head * head_dim, q_bf16);
                LOAD_CONV_SILU(k, key_offset + head * head_dim, k_bf16);

                const float64_pair_t q_square =
                    v_bf16_mul_acc32_b(q_bf16, q_bf16);
                float64 q_norm = q_square.v1 + q_square.v2;
                q_norm = v_f32_reduce_add(q_norm);
                q_norm = v_f32_shuffle_b(
                    q_norm, broadcast_lane_zero, 0, q_norm);
                q_norm = v_rsqrt_fast_f32(q_norm + 1e-6f);

                const float64_pair_t k_square =
                    v_bf16_mul_acc32_b(k_bf16, k_bf16);
                float64 k_norm = k_square.v1 + k_square.v2;
                k_norm = v_f32_reduce_add(k_norm);
                k_norm = v_f32_shuffle_b(
                    k_norm, broadcast_lane_zero, 0, k_norm);
                k_norm = v_rsqrt_fast_f32(k_norm + 1e-6f);

                const float64_pair_t q_f32 =
                    v_convert_bf16_to_f32_all_b(q_bf16);
                const float64_pair_t k_f32 =
                    v_convert_bf16_to_f32_all_b(k_bf16);
                float64_pair_t q_normalized;
                q_normalized.v1 = q_f32.v1 * q_norm;
                q_normalized.v2 = q_f32.v2 * q_norm;
                float64_pair_t k_normalized;
                k_normalized.v1 = k_f32.v1 * k_norm;
                k_normalized.v2 = k_f32.v2 * k_norm;

                const bfloat128 q_result =
                    v_convert_f32_to_bf16_all_b(q_normalized);
                const bfloat128 k_result =
                    v_convert_f32_to_bf16_all_b(k_normalized);

                const int output_head = head * 3;
                output_coords[1] = output_head;
                output_coords[0] = 0;
                v_bf16_st_tnsr(output_coords, q_out, q_result);
                v_bf16_st_tnsr(output_coords, k_out, k_result);
                output_coords[1] = output_head + 1;
                v_bf16_st_tnsr(output_coords, q_out, q_result);
                v_bf16_st_tnsr(output_coords, k_out, k_result);
                output_coords[1] = output_head + 2;
                v_bf16_st_tnsr(output_coords, q_out, q_result);
                v_bf16_st_tnsr(output_coords, k_out, k_result);
            }

            bfloat128 value;
            LOAD_CONV_SILU(v, value_offset + head * head_dim, value);
            output_coords[0] = 0;
            output_coords[1] = head;
            v_bf16_st_tnsr(output_coords, v_out, value);
        }
    }
}

#undef DECLARE_CONV_WEIGHTS
#undef LOAD_CONV_SILU
#undef APPLY_SILU

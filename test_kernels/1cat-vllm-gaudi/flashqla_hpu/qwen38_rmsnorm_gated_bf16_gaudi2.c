// Qwen3.8 GDN output RMSNorm + SiLU gate for Gaudi2.
//
// Each index-space point owns one 128-element row.  Keep the RMS reduction,
// normalization, SiLU gate, and BF16 conversion in a single TPC pass so the
// 192 MiB intermediates never round-trip through HBM.  Gaudi2's BF16 sigmoid
// LUT avoids the much slower software-expanded FP32 exponential; all products
// around that one approximation remain FP32.
void main(tensor x, tensor z, tensor weight, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const uchar256 broadcast_lane_zero = 0x80;

    int5 row_coords = {0, 0, 0, 0, 0};
    const int5 weight_coords = {0, 0, 0, 0, 0};
    const bfloat128 weight_bf16 =
        v_bf16_ld_tnsr_b(weight_coords, weight);
    const float64_pair_t weight_f32 =
        v_convert_bf16_to_f32_all_b(weight_bf16);

    #pragma loop_unroll(4) pipelined taken
    for (int row = start[0]; row < end[0]; ++row)
    {
        row_coords[1] = row;
        const bfloat128 x_bf16 = v_bf16_ld_tnsr_b(row_coords, x);
        const bfloat128 z_bf16 = v_bf16_ld_tnsr_b(row_coords, z);

        const float64_pair_t x_square =
            v_bf16_mul_acc32_b(x_bf16, x_bf16);
        float64 inverse_rms = x_square.v1 + x_square.v2;
        inverse_rms = v_f32_reduce_add(inverse_rms);
        inverse_rms = v_f32_shuffle_b(
            inverse_rms,
            broadcast_lane_zero,
            0,
            inverse_rms);
        inverse_rms = v_rsqrt_fast_f32(
            inverse_rms * (1.0f / 128.0f) + 1e-6f);

        const float64_pair_t x_f32 =
            v_convert_bf16_to_f32_all_b(x_bf16);
        const float64_pair_t z_f32 =
            v_convert_bf16_to_f32_all_b(z_bf16);
        float64_pair_t output_f32;

        #ifdef QWEN38_RMSNORM_GATED_IDENTITY_GATE
        const float64 gate_low = z_f32.v1;
        const float64 gate_high = z_f32.v2;
        #else
        const float64 sigmoid_low =
            no_saturation_sigmoid_f32(z_f32.v1);
        const float64 sigmoid_high =
            no_saturation_sigmoid_f32(z_f32.v2);
        const float64 gate_low = z_f32.v1 * sigmoid_low;
        const float64 gate_high = z_f32.v2 * sigmoid_high;
        #endif

        output_f32.v1 =
            x_f32.v1 * inverse_rms * weight_f32.v1 * gate_low;
        output_f32.v2 =
            x_f32.v2 * inverse_rms * weight_f32.v2 * gate_high;
        v_bf16_st_tnsr(
            row_coords,
            output,
            v_convert_f32_to_bf16_all_b(output_f32));
    }
}

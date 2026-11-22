// SPDX-License-Identifier: Apache-2.0
// Consume raw gate logits.  This candidate is retained for isolated numerical
// and performance experiments; the production exact path continues to use the
// stock softplus/sqrt followed by the exact top-6 kernel.
static inline float64 router_stock_exp_cephes(float64 input) {
    // The deployed Gaudi2 softplus kernel predates tpc-special.h's current
    // six-coefficient exp_cephes implementation.  Reproduce its four-
    // coefficient/five-MAC sequence so fused routing remains bitwise exact.
    const float ln_2_1 = 0.693359375f;
    const float ln_2_2 = -2.12194440e-4f;
    const float log2_e = 1.44269504088896341f;
    const float64 c6 = .49998858346161135f;
    const float64 c5 = 0.1666634537038239f;
    const float64 c4 = 4.191878872870153e-2f;
    const float64 c3 = 8.380148765026943e-3f;

    float64 z = v_f32_mac_b(input, log2_e, 0.5f, 0);
    z = v_f32_nearbyint_b(z, SW_RD);
    float64 reduced = v_f32_mac_b(z, ln_2_1, input, SW_NEG);
    reduced = v_f32_mac_b(z, ln_2_2, reduced, SW_NEG);
    float64 result = v_f32_mac_b(reduced, c3, c4, 0);
    result = v_f32_mac_b(result, reduced, c5, 0);
    result = v_f32_mac_b(result, reduced, c6, 0);
    result = v_f32_mac_b(result, reduced, 1.0f, 0);
    result = v_f32_mac_b(result, reduced, 1.0f, 0);

    int64 bits = *((int64*)&result);
    int64 exponent = v_convert_f32_to_i32_b(z, SW_RD);
    exponent = v_i32_shl_b(exponent, 23);
    bits = v_i32_add_b(bits, exponent);
    result = *((float64*)&bits);

    const float64 lower = -87.336f;
    const float64 upper = 88.722f;
    const int64 plus_inf_bits = 0x7f800000;
    result = v_f32_sel_leq_f32_b(input, lower, 0.0f, result);
    result = v_f32_sel_geq_f32_b(
        input, upper, *((float64*)&plus_inf_bits), result);
    result = v_f32_sel_grt_u32_b(
        *((uint64*)&input) & 0x7fffffff, 0x7f800000, input, result);
    return result;
}

static inline float64 router_stock_sqrt(float64 input) {
    // Match the Gaudi2 sqrt_fwd_f32 kernel rather than tpc-special.h's legacy
    // v_sqrt_f32 helper.  The stock kernel uses the hardware PRE/POST-SQRT
    // normalization around the coefficient-table polynomial.  The legacy
    // helper extracts and reconstructs the exponent in integer vectors and can
    // differ by one FP32 ULP on real router scores.
    const int FUNC_ID = e_fp32_sqrt;
    float64 result = v_f32_form_fp_num_b(
        input, input, input, SW_FORCE_SIGN0 | SW_PRE_SQRT_RSQRT);
    const uint64_float64_pair_t lut =
        v_f32_get_lut_entry_and_interval_start_b(
            result, 17, e_func_variant_sqrt_rsqrt << 13,
            (uint64_float64_pair_t){0}, 1, 0);
    const uint64 intervals = lut.v1;
    const float64 reduced = result - lut.v2;
    float64 c0 = v_f32_lookup_1c(
        intervals, FUNC_ID, SW_BV32, 0, 1, 0);
    const float64_pair_t c1c2 = v_f32_lookup_2c(
        intervals, FUNC_ID, SW_BV32, (float64_pair_t){0}, 1, 0);
    result = c1c2.v1;
    result = v_f32_mac_b(c1c2.v2, reduced, result, 0, 1, 0);
    result = v_f32_mac_b(result, reduced, c0, 0, 1, 0);
    result = v_f32_form_fp_num_b(
        input, result, result, SW_FORCE_SIGN0 | SW_POST_SQRT);
    const float64 fclass = v_f32_fclass_b(input);
    result = v_f32_calc_fp_special_b(
        fclass, fclass, e_fp_sqrt, result);
    return result;
}

static inline float64 router_score(float64 logits) {
    // Match Gaudi's stock softplus_fwd_f32 exactly.  The vendor kernel uses
    // log1p_f32(exp_cephes_f32(x)); log1p_f32 selects the quadratic
    // x * (1 - 0.5 * x) for |x| < 5e-3 instead of evaluating log(1 + x).
    // Keeping that branch is required for bitwise-identical router weights on
    // the negative-logit lanes; the previous plain log path differed by one
    // FP32 ULP on real model inputs.
    const float64 exponential = router_stock_exp_cephes(logits);
    const float64 small = v_f32_mul_b(
        exponential,
        v_f32_mac_b(exponential, 0.5f, (float64)1.0f, SW_NEG));
    const bool64 use_small = v_f32_cmp_less_b(exponential, 5.0e-3f);
    const float64 regular = v_log_f32(1.0f + exponential);
    const float64 softplus = v_f32_mov_vb(
        small, 0, regular, use_small, 0);
    // The Gaudi2 stock softplus_fwd_f32 kernel always returns
    // log1p_f32(exp_cephes_f32(x)).  Unlike the generic CPU definition it
    // does not replace values above 20 with x.  Keeping that extra threshold
    // here changes selected routing weights by one FP32 ULP on real logits.
    return router_stock_sqrt(softplus);
}

void main(tensor logits, tensor text_bias, tensor image_bias, tensor image_mask,
#ifdef DSV41_ROUTER_SHARED_SCALED
          tensor channel_scale, tensor activation_scale,
#endif
          tensor output_ids, tensor output_weights) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int64 lanes = (int64)V_LANE_ID_32;
    for (int token = start[0]; token < end[0]; ++token) {
        const int5 image_at = {token, 0, 0, 0, 0};
        const char is_image = s_i8_ld_g(gen_addr(image_at, image_mask));
#ifdef DSV41_ROUTER_SHARED_SCALED
#ifdef DSV41_ROUTER_READY_PAIR
        const int first = 0;
        const int rows = get_dim_size(logits, 1) / 2;
#else
        const int first = get_dim_size(logits, 0) - 512;
#endif
        const float sx = s_f32_ld_g(gen_addr((int5){0, token}, activation_scale));
#endif
        float64 scores[6], choices[6];
        float64 packed_weights = 0;
        int64 packed_ids = 0;
        bool64 eligible[6];
        float64 sum = 0;
        #pragma unroll
        for (int c = 0; c < 6; ++c) {
#if !defined(DSV41_ROUTER_SHARED_SCALED) && !defined(DSV41_ROUTER_PARTIAL_LOGITS)
            const int5 at = {c * 64, token, 0, 0, 0};
#endif
            const int5 bias_at = {c * 64, 0, 0, 0, 0};
#ifdef DSV41_ROUTER_SHARED_SCALED
            float64 projected = v_f32_ld_tnsr_b((int5){first + c * 64, token}, logits);
            projected = v_f32_mul_b(projected, v_f32_ld_tnsr_b((int5){c * 64, 0}, channel_scale));
#ifdef DSV41_ROUTER_READY_PAIR
            const float64 low_scale = v_f32_ld_tnsr_b((int5){c * 64, 1}, channel_scale);
            projected += v_f32_ld_tnsr_b((int5){512 + c * 64, token}, logits) * low_scale;
            float64 low_input = v_f32_ld_tnsr_b((int5){c * 64, token + rows}, logits) *
                v_f32_ld_tnsr_b((int5){c * 64, 0}, channel_scale);
            low_input += v_f32_ld_tnsr_b((int5){512 + c * 64, token + rows}, logits) * low_scale;
            projected += low_input * .0625f;
#endif
            projected = v_f32_mul_b(projected, sx);
            scores[c] = router_score(projected);
#else
#ifdef DSV41_ROUTER_PARTIAL_LOGITS
            float64 projected=v_f32_ld_tnsr_b((int5){c*64,token,0},logits);
            projected+=v_f32_ld_tnsr_b((int5){c*64,token,1},logits);
            projected+=v_f32_ld_tnsr_b((int5){c*64,token,2},logits);
            projected+=v_f32_ld_tnsr_b((int5){c*64,token,3},logits);
            scores[c]=router_score(projected);
#else
            scores[c] = router_score(v_f32_ld_tnsr_b(at, logits));
#endif
#endif
            const float64 bias = is_image
                ? v_f32_ld_tnsr_b(bias_at, image_bias)
                : v_f32_ld_tnsr_b(bias_at, text_bias);
            choices[c] = scores[c] + bias;
            eligible[c] = (bool64)1;
        }
        #pragma unroll
        for (int rank = 0; rank < 6; ++rank) {
            int64_float64_pair_t local;
            local.v1 = 2147483647;
            local.v2 = as_float64((int64)0xff800000);
            float64 local_score = 0;
            #pragma unroll
            for (int c = 5; c >= 0; --c) {
                local_score = v_f32_sel_geq_f32_vb(
                    choices[c], local.v2, scores[c], local_score,
                    0, local_score, eligible[c], 0);
                local = v_i32_sel2_geq_f32_vb(
                    choices[c], local.v2, lanes + c * 64, local.v1,
                    0, local, eligible[c], 0);
            }
            const float64 best = v_f32_reduce_max(local.v2);
            int64 winner = v_i32_sel_eq_f32_b(
                local.v2, best, local.v1, 2147483647);
            winner = v_i32_reduce_min(winner);
            const bool64 selected_lane = v_i32_cmp_eq_b(local.v1, winner);
            const float64 selected = v_f32_mov_vb(
                local_score, 0, 0, selected_lane, 0);
            #pragma unroll
            for (int c = 0; c < 6; ++c)
                eligible[c] = eligible[c] &
                    v_i32_cmp_neq_b(lanes + c * 64, winner);
            const float64 selected_weight = v_f32_reduce_add(selected);
            const bool64 output_lane = v_i32_cmp_eq_b(lanes, rank);
            packed_weights = v_f32_mov_vb(
                selected_weight, 0, packed_weights, output_lane, 0);
            packed_ids = v_i32_mov_vb(
                winner, 0, packed_ids, output_lane, 0);
            sum += selected_weight;
        }
        const float64 denominator = sum + 1.0e-20f;
        float64 inverse = v_reciprocal_f32(denominator);
        inverse = inverse * (2.0f - denominator * inverse);
        const int5 at = {0, token, 0, 0, 0};
        v_i32_st_tnsr_partial(at, output_ids, packed_ids, 5, 0);
        v_f32_st_tnsr_partial(
            at, output_weights, packed_weights * inverse * 1.5f, 5, 0);
    }
}

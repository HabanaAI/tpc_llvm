// SPDX-License-Identifier: Apache-2.0
// DeepSeek V4.1 main-KV FP4/E4M3FN: page lookup, nibble expansion and
// BF16 conversion in one TPC pass. Four 128-wide vectors form one KV row.
void main(tensor packed, tensor pages, tensor logical_rows, tensor output, int ratio) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int page_rows = ratio ? 128 / ratio : 0;
    const int page_count = get_dim_size(pages, 0);
    const int cache_rows = get_dim_size(packed, 1);
    const uchar256 lane8 = V_LANE_ID_8;
    const ushort128 lane16 = V_LANE_ID_16;
    for (int out_row = begin[0]; out_row < end[0]; ++out_row) {
        bool valid = out_row < cache_rows;
        int physical = out_row;
        if (ratio != 0) {
            const int logical = s_i32_ld_g(gen_addr((int5){out_row}, logical_rows));
            valid = logical >= 0 && logical < page_count * page_rows;
            physical = 0;
            if (valid) {
                const int page = s_i32_ld_g(gen_addr((int5){logical / page_rows}, pages));
                const int offset = logical % page_rows;
                valid = page >= 0 && offset < cache_rows && page <= (cache_rows - 1 - offset) / page_rows;
                if (valid) physical = page * page_rows + offset;
            }
        }
        for (int chunk = 0; chunk < 4; ++chunk) {
            uchar256 raw = valid ? v_u8_ld_tnsr_partial_b(
                (int5){chunk * 64, physical}, packed, 63, 0) : (uchar256)0;
            raw = v_u8_mov_dual_group_all_b(raw, 0xffffffff, 0, 0, 0, 0,
                                             MkWrA(3, 3, 3, 3), (uchar256)0);
            const uchar256 expanded = v_u8_shuffle_b(raw, (lane8 >> 1) | 0x80, 0, (uchar256)0);
            const ushort128 codes = (convert_uchar256_to_ushort256(expanded, SW_LINEAR).v1 >>
                                     ((lane16 & 1) << 2)) & 15;
            const ushort128 magnitude = codes & 7;
            ushort128 value_bits = (magnitude << 6) + 0x3f00;
            value_bits = v_u16_sel_eq_u16_b(magnitude, 1, 0x3f00, value_bits);
            value_bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, value_bits);
            value_bits |= (codes & 8) << 12;

            uchar256 scale_bytes = valid ? v_u8_ld_tnsr_partial_b(
                (int5){256 + chunk * 8, physical}, packed, 7, 0) : (uchar256)0;
            scale_bytes = v_u8_mov_dual_group_all_b(scale_bytes, 0xffffffff,
                0, 0, 0, 0, MkWrA(3, 3, 3, 3), (uchar256)0);
            scale_bytes = v_u8_shuffle_b(scale_bytes, (lane8 >> 4) | 0x80, 0, (uchar256)0);
            const ushort128 scale_codes = convert_uchar256_to_ushort256(scale_bytes, SW_LINEAR).v1;
            const ushort128 exponent = (scale_codes >> 3) & 15;
            const ushort128 mantissa = scale_codes & 7;
            ushort128 tiny = (mantissa << 5) + 0x3b80;
            tiny = v_u16_sel_less_u16_b(mantissa, 4, (mantissa << 6) + 0x3b00, tiny);
            tiny = v_u16_sel_eq_u16_b(mantissa, 1, 0x3b00, tiny);
            tiny = v_u16_sel_eq_u16_b(mantissa, 0, 0, tiny);
            ushort128 scale_bits = ((exponent + 120) << 7) | (mantissa << 4);
            scale_bits = v_u16_sel_eq_u16_b(exponent, 0, tiny, scale_bits);
            scale_bits |= (scale_codes & 128) << 8;
            scale_bits = v_u16_sel_eq_u16_b(scale_codes & 127, 127, 0x7fff, scale_bits);
            const bfloat128 product = v_bf16_mul_b(
                *((bfloat128*)&value_bits), *((bfloat128*)&scale_bits));
            // The production HPU unpack path canonicalizes both signs of
            // exact zero before the BF16 store. Preserve that device contract.
            const bfloat128 result = v_bf16_sel_eq_bf16_b(
                product, (bfloat)0, (bfloat)0, product);
            v_bf16_st_tnsr((int5){chunk * 128, out_row}, output, result);
        }
    }
}

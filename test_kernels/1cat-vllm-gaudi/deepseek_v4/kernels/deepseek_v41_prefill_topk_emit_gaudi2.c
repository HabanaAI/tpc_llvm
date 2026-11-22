// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_score_order.h"
static inline unsigned score_key(float value) {
#if DSV41_SCORE_KEY_BITS == 32
    unsigned bits = *((unsigned*)&value);
    const unsigned magnitude = bits & 0x7fffffffu;
    if (magnitude > 0x7f800000u) return 0xffffffffu;
    if (!magnitude) bits = 0;
    return bits > 0x7fffffffu ? ~bits : bits ^ 0x80000000u;
#else
    unsigned bits = *((unsigned*)&value) >> 16;
    const unsigned magnitude = bits & 32767;
    if (magnitude > 32640) return 65535;
    if (!magnitude) bits = 0;
    return bits > 32767 ? (~bits) & 65535 : bits ^ 32768;
#endif
}
void main(tensor scores, tensor metadata, tensor values, tensor indices, int columns, int width) {
    const int5 begin = get_index_space_offset(), end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row) {
        if (width == columns) {
            for (int worker = begin[0]; worker < end[0]; ++worker)
                for (int col = worker * 64; col < columns; col += 8 * 64) {
                    const int5 at = {col,row};
                    v_f32_st_tnsr(at, values, v_f32_ld_tnsr_b(at,scores));
                    v_i32_st_tnsr(at, indices, (int64)V_LANE_ID_32 + col);
                }
            continue;
        }
#if DSV41_SCORE_KEY_BITS != 32
        const unsigned threshold = s_u32_ld_g(gen_addr((int5){0,row},metadata));
#endif
        const int greater_total = s_i32_ld_g(gen_addr((int5){1,row},metadata));
        const int budget = width - greater_total;
        for (int worker = begin[0]; worker < end[0]; ++worker) {
            const int greater = s_i32_ld_g(gen_addr((int5){2 + worker * 2,row},metadata));
            int equal = s_i32_ld_g(gen_addr((int5){3 + worker * 2,row},metadata));
            int written = greater + (equal < budget ? equal : budget);
            const int first = columns * worker / 8, last = columns * (worker + 1) / 8;
#if DSV41_SCORE_KEY_BITS == 32
            const int word_capacity = (columns + 127) / 128 * 4;
            for (int word = first / 32; word * 32 < last; ++word) {
                unsigned greater_bits = s_u32_ld_g(gen_addr((int5){18 + word,row},metadata));
                unsigned equal_bits = s_u32_ld_g(gen_addr((int5){18 + word_capacity + word,row},metadata));
                unsigned active = 0xffffffffu;
                if (word == first / 32) active <<= first % 32;
                if ((word + 1) * 32 > last) active &= 0xffffffffu >> (32 - last % 32);
                unsigned keep = greater_bits & active;
                equal_bits &= active;
                while (equal_bits && equal < budget) {
                    keep |= equal_bits & -equal_bits;
                    equal_bits &= equal_bits - 1;
                    ++equal;
                }
                while (keep) {
                    const int col = word * 32 + s_u32_find_first(keep,SW_FIND_ONE|SW_LSB);
                    const float score = s_f32_ld_g(gen_addr((int5){col,row},scores));
                    const int5 at = {written++,row};
                    s_f32_st_g(gen_addr(at,values),score);
                    s_i32_st_g(gen_addr(at,indices),col);
                    keep &= keep - 1;
                }
            }
#else
            for (int col = first; col < last; ++col) {
                const float score = s_f32_ld_g(gen_addr((int5){col,row},scores));
                const unsigned key = score_key(score);
                if (key > threshold || (key == threshold && equal++ < budget)) {
                    const int5 at = {written++,row};
                    s_f32_st_g(gen_addr(at,values),score);
                    s_i32_st_g(gen_addr(at,indices),col);
                }
            }
#endif
        }
    }
}

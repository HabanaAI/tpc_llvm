// SPDX-License-Identifier: Apache-2.0
// Diagnostic only: identical route/N/K128 partition and eight-load schedule.
// Read/decode probes retain one XOR checksum per K128 tile so loads cannot
// disappear. The checksum vector cost is shared; it is not a pure-read timer.
void main(tensor ids, tensor q16, tensor planes, tensor lookup, tensor output, int route_pack)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
#if DSV41_PROBE_MODE == 2
    const uchar256 bits = 0x38;
    const minifloat256 ones = *((minifloat256*)&bits);
    for (int slot = start[1]; slot < end[1]; ++slot)
        for (int block = start[0]; block < end[0]; ++block)
            for (int k = start[2] * 128; k < end[2] * 128; ++k)
                v_f8_st_tnsr((int5){block * 256, k, slot}, output, ones);
#else
    const int blocks = get_dim_size(q16, 1);
#if DSV41_PROBE_MODE == 1
    const uchar256 raw_table = v_u8_ld_tnsr_b((int5){0}, lookup);
    const uchar256 table = v_u8_sel_eq_u8_b(raw_table, 127, 0, raw_table + 48, SW_MASK_EQ_ZERO);
#endif
    for (int slot = start[1]; slot < end[1]; ++slot) {
        for (int block = start[0]; block < end[0]; ++block) {
            const int source_block = block % blocks;
            const int expert = s_i32_ld_g(gen_addr((int5){slot * route_pack + block / blocks}, ids));
#if DSV41_PROBE_MODE == 1
            const uchar256 channel = v_u8_ld_tnsr_b(
                (int5){get_dim_size(planes, 0) - 128, source_block, expert}, planes);
#endif
            for (int tile = start[2]; tile < end[2]; ++tile) {
                uchar256 ca = 0, cb = 0, cc = 0, cd = 0, ce = 0, cf = 0, cg = 0, ch = 0;
                for (int group = tile * 4; group < tile * 4 + 4; ++group) {
#if DSV41_PROBE_MODE == 1
                    const uchar256 stored = v_u8_ld_tnsr_b((int5){group * 128, source_block, expert}, planes);
                    const uchar256 subtract = (uchar256)48 - ((stored - channel) << 3);
#endif
                    int5 source = {group * 2048, source_block, expert};
                    for (int chunk = 0; chunk < 4; ++chunk) {
                        uchar256 a = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
                        uchar256 b = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
                        uchar256 c = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
                        uchar256 d = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
                        uchar256 e = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
                        uchar256 f = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
                        uchar256 g = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
                        uchar256 h = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); source[0] += 64;
#if DSV41_PROBE_MODE == 1
#define VALUE(X) v_u8_sub_b(v_u8_shuffle_b(table, (X) | 0x80, 0, (X)), subtract, SW_SAT)
#else
#define VALUE(X) (X)
#endif
                        ca ^= VALUE(a); cb ^= VALUE(b); cc ^= VALUE(c); cd ^= VALUE(d);
                        ce ^= VALUE(e); cf ^= VALUE(f); cg ^= VALUE(g); ch ^= VALUE(h);
#undef VALUE
                    }
                }
                v_u8_st_tnsr((int5){block * 256, tile, slot}, output, ca ^ cb ^ cc ^ cd ^ ce ^ cf ^ cg ^ ch);
            }
        }
    }
#endif
}

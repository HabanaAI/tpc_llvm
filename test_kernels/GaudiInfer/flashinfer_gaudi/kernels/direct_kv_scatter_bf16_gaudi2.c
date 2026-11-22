// SPDX-License-Identifier: Apache-2.0
// g2e Direct MME+SRAM KV cache scatter (ZERO per-round patching!):
//   Copies 4 slots (slot_base..slot_base+3) x 4 heads x 16 tokens x 256 from SRAM k_sram/v_sram
//   into fixed-base HBM per-slot KV caches [256, cap, 4] (stride[2] = cap*256), reading each slot's
//   dynamic start position `pos[b] = last_tab[0, b]` directly from HBM!
// Index space: dim 1 = idx in [0..256) where s = idx >> 6 (0..3), h = (idx >> 4) & 3, t = idx & 15.

void main(tensor k_sram, tensor v_sram,
          tensor k0, tensor k1, tensor k2, tensor k3,
          tensor v0, tensor v1, tensor v2, tensor v3,
          tensor last_tab,
          int slot_base)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    if (slot_base < 0) {
        // B=1 head-addressed mode: k0..k3 and v0..v3 are heads 0..3 of slot 0 ([256, max_rows]),
        // completely independent of slot cap! Index space: dim 1 = idx in [0..64), h = idx >> 4, t = idx & 15.
        int5 l0 = {0, 0, 0, 0, 0};
        const int p = s_i32_ld_g((__global__ int*)gen_addr(l0, last_tab));
        if (p < 0) return;
        for (int idx = start[1]; idx < end[1]; ++idx) {
            const int h = idx >> 4;
            const int t = idx & 15;
            int5 sc0 = {0, t, h, 0, 0};
            int5 dc0 = {0, p + t, 0, 0, 0};
#ifdef KV_FP8
            const minifloat256 kv = v_f8_ld_tnsr_b(sc0, k_sram);
            const minifloat256 vv = v_f8_ld_tnsr_b(sc0, v_sram);
            if (h == 0) { v_f8_st_tnsr(dc0, k0, kv); v_f8_st_tnsr(dc0, v0, vv); }
            else if (h == 1) { v_f8_st_tnsr(dc0, k1, kv); v_f8_st_tnsr(dc0, v1, vv); }
            else if (h == 2) { v_f8_st_tnsr(dc0, k2, kv); v_f8_st_tnsr(dc0, v2, vv); }
            else { v_f8_st_tnsr(dc0, k3, kv); v_f8_st_tnsr(dc0, v3, vv); }
#else
            int5 sc1 = {128, t, h, 0, 0};
            int5 dc1 = {128, p + t, 0, 0, 0};
            const bfloat128 ka = v_bf16_ld_tnsr_b(sc0, k_sram);
            const bfloat128 kb = v_bf16_ld_tnsr_b(sc1, k_sram);
            const bfloat128 va = v_bf16_ld_tnsr_b(sc0, v_sram);
            const bfloat128 vb = v_bf16_ld_tnsr_b(sc1, v_sram);
            if (h == 0) {
                v_bf16_st_tnsr(dc0, k0, ka); v_bf16_st_tnsr(dc1, k0, kb);
                v_bf16_st_tnsr(dc0, v0, va); v_bf16_st_tnsr(dc1, v0, vb);
            } else if (h == 1) {
                v_bf16_st_tnsr(dc0, k1, ka); v_bf16_st_tnsr(dc1, k1, kb);
                v_bf16_st_tnsr(dc0, v1, va); v_bf16_st_tnsr(dc1, v1, vb);
            } else if (h == 2) {
                v_bf16_st_tnsr(dc0, k2, ka); v_bf16_st_tnsr(dc1, k2, kb);
                v_bf16_st_tnsr(dc0, v2, va); v_bf16_st_tnsr(dc1, v2, vb);
            } else {
                v_bf16_st_tnsr(dc0, k3, ka); v_bf16_st_tnsr(dc1, k3, kb);
                v_bf16_st_tnsr(dc0, v3, va); v_bf16_st_tnsr(dc1, v3, vb);
            }
#endif
        }
        return;
    }
    int pos0, pos1, pos2, pos3;
    {
        int5 l0 = {0, slot_base + 0, 0, 0, 0};
        int5 l1 = {0, slot_base + 1, 0, 0, 0};
        int5 l2 = {0, slot_base + 2, 0, 0, 0};
        int5 l3 = {0, slot_base + 3, 0, 0, 0};
        pos0 = s_i32_ld_g((__global__ int*)gen_addr(l0, last_tab));
        pos1 = s_i32_ld_g((__global__ int*)gen_addr(l1, last_tab));
        pos2 = s_i32_ld_g((__global__ int*)gen_addr(l2, last_tab));
        pos3 = s_i32_ld_g((__global__ int*)gen_addr(l3, last_tab));
    }
    for (int idx = start[1]; idx < end[1]; ++idx) {
        const int s = idx >> 6;
        const int h = (idx >> 4) & 3;
        const int t = idx & 15;
        const int b = slot_base + s;
        const int p = (s == 0) ? pos0 : ((s == 1) ? pos1 : ((s == 2) ? pos2 : pos3));
        if (p < 0) continue;
        int5 sc0 = {0, t, h, b, 0};
        int5 dc0 = {0, p + t, h, 0, 0};
#ifdef KV_FP8
        const minifloat256 kv = v_f8_ld_tnsr_b(sc0, k_sram);
        const minifloat256 vv = v_f8_ld_tnsr_b(sc0, v_sram);
        if (s == 0) { v_f8_st_tnsr(dc0, k0, kv); v_f8_st_tnsr(dc0, v0, vv); }
        else if (s == 1) { v_f8_st_tnsr(dc0, k1, kv); v_f8_st_tnsr(dc0, v1, vv); }
        else if (s == 2) { v_f8_st_tnsr(dc0, k2, kv); v_f8_st_tnsr(dc0, v2, vv); }
        else { v_f8_st_tnsr(dc0, k3, kv); v_f8_st_tnsr(dc0, v3, vv); }
#else
        int5 sc1 = {128, t, h, b, 0};
        int5 dc1 = {128, p + t, h, 0, 0};
        const bfloat128 ka = v_bf16_ld_tnsr_b(sc0, k_sram);
        const bfloat128 kb = v_bf16_ld_tnsr_b(sc1, k_sram);
        const bfloat128 va = v_bf16_ld_tnsr_b(sc0, v_sram);
        const bfloat128 vb = v_bf16_ld_tnsr_b(sc1, v_sram);
        if (s == 0) {
            v_bf16_st_tnsr(dc0, k0, ka); v_bf16_st_tnsr(dc1, k0, kb);
            v_bf16_st_tnsr(dc0, v0, va); v_bf16_st_tnsr(dc1, v0, vb);
        } else if (s == 1) {
            v_bf16_st_tnsr(dc0, k1, ka); v_bf16_st_tnsr(dc1, k1, kb);
            v_bf16_st_tnsr(dc0, v1, va); v_bf16_st_tnsr(dc1, v1, vb);
        } else if (s == 2) {
            v_bf16_st_tnsr(dc0, k2, ka); v_bf16_st_tnsr(dc1, k2, kb);
            v_bf16_st_tnsr(dc0, v2, va); v_bf16_st_tnsr(dc1, v2, vb);
        } else {
            v_bf16_st_tnsr(dc0, k3, ka); v_bf16_st_tnsr(dc1, k3, kb);
            v_bf16_st_tnsr(dc0, v3, va); v_bf16_st_tnsr(dc1, v3, vb);
        }
#endif
    }
}

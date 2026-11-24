// SPDX-License-Identifier: Apache-2.0
// Multi-token-prediction verification of a Gated DeltaNet layer in WY form, for
// T = 16 draft tokens, 16 key and 48 value heads of 128, batch 1, one value head
// per member. Phase 1 computes the delta-rule updates ds_t and the outputs out_t
// of all tokens from the initial state's products with each k_t and q_t (rmat)
// and the Gram matrix of the keys and queries (gram), both precomputed on the
// MME: per-token coefficient vectors in local memory, lane broadcasts and MAC
// chains. Phase 2 streams 8-row tiles of the state through all 16 tokens,
// S_t = g_t S_{t-1} + ds_t k_t^T, and stores every S_t (a 50 MB checkpoint
// output). The inputs are independent random values, so gram is not the Gram
// matrix of kvals; it is positive, so the rsqrt of its diagonal is defined.
// From GaudiInfer (https://github.com/JChen-318/GaudiInfer, commit cb260a91),
// tpc/flashinfer_gaudi/kernels/gdn_mtp_wy_f32_gaudi2.c; Apache-2.0.
//
// BENCH: arch gaudi2
// BENCH: in  f32 128 128 48 1 range=-1,1
// BENCH: in  f32 128 16 16 1 range=-1,1
// BENCH: in  f32 48 16 1 range=-0.5,0
// BENCH: in  bf16 48 16 1 range=0,1
// BENCH: in  f32 128 32 48 1 range=-1,1
// BENCH: in  f32 32 32 16 1 range=0.5,1
// BENCH: in  f32 128 48 16 1 range=-1,1
// BENCH: out f32 128 48 16 1
// BENCH: out f32 128 128 48 16 1
// BENCH: index-space 1 1 48 1

// WY-form full-query MTP verification core (B<=16, T=TOKENS (8 or 16), H=16, HV=48, K=V=128).
//
// Same recurrence and outputs as gdn_mtp_fused_f32_gaudi2.c, but the dot products of the evolving state with every
// k_t and q_t are not computed here. The caller precomputes on the MME (FP32 matmuls):
//   rmat [B, 48, 2T, V] : rows t < T: S0 k_t (raw k), rows T + t: S0 q_t (raw q)     (S0: gathered initial state)
//   gram [B, 16, 2T, 2T]: X X^T for X = [k_0..k_{T-1}; q_0..q_{T-1}] (raw), one per key head
//   kvals [B, T, 16, K] : raw k in FP32;  vals [B, T, 48, V]: v in FP32
// With rk = 1/|k|, rq = scale/|q| (as the fused kernel), G_t = prod_{i<=t} g_i and ds_t = rk_t delta_t:
//   ds_t  = rk_t beta_t v_t - beta_t G_t rk_t^2 (S0 k_t) - sum_{j<t} beta_t (G_t/G_j) rk_t^2 (k_t.k_j) ds_j
//   out_t = G_t rq_t (S0 q_t) + sum_{j<=t} (G_t/G_j) rq_t (q_t.k_j) ds_j
//   S_t   = g_t S_{t-1} + ds_t k_t^T                       (the checkpoint update of the fused kernel)
// Phase 1 (per head, all 128 rows as two 64-lane vectors): ds_t and out_t; out_t is stored here.
// Phase 2 (tiles of ROWS rows carried in registers across all tokens; each state row is read once): only the
// state update and checkpoint stores. Lanes are moved with lane-XOR swaps (the fused kernel's reduction moves),
// so every loop over tokens is a runtime loop: small code, low register pressure.
#define ROWS 8
#ifndef TOKENS
#define TOKENS 16
#endif
#if TOKENS != 8 && TOKENS != 16
#error "TOKENS must be 8 or 16"
#endif
// lanes of a coefficient vector: [0, T) -a_tj, [T, 2T) e_tj, then per-token scalars
#define L_RKB (2 * TOKENS)        // rk_t beta_t
#define L_B (2 * TOKENS + 1)      // beta_t G_t rk_t^2
#define L_C (2 * TOKENS + 2)      // G_t rq_t
#define L_G (2 * TOKENS + 3)      // g_t

#define XOR_LUT(name, m)                                                                                       \
    uint64 name##_bytes = 0x80808080;                                                                          \
    name##_bytes += v_u32_xor_b(read_lane_id_4b_b() * 0x01010101, (m) * 0x01010101);                          \
    name##_bytes = v_u32_mov_group_b(name##_bytes, 0xFFFFFFFF, (SW_GROUP1_EN | SW_DUAL_GROUP0_EN), name##_bytes, \
                                     1, 0);                                                                    \
    name##_bytes = v_u32_add_vb(name##_bytes, 0x20202020, 0, name##_bytes, to_bool64(pred_group_1));           \
    name##_bytes = v_u32_mov_dual_group_all_b(name##_bytes, 0xFFFFFFFF, 0, 0, 0, 0,                            \
                                              MkWrA(0b11, 0b11, 0b11, 0b11), 0);                               \
    const uchar256 name = (uchar256)name##_bytes;

#define XOR8(v) v_f32_mov_group_b((v), 0xFFFFFFFF, SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN |           \
                                  SW_DUAL_GROUP1_EN | SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN, (v), 1, 0)
#define XOR16(v) v_f32_mov_dual_group_all_b((v), 0xFFFFFFFF, 1, 0, 3, 2, MkWrA(0b11, 0b11, 0b11, 0b11), (v))
#define XOR32(v) v_f32_mov_dual_group_all_b((v), 0xFFFFFFFF, 2, 3, 0, 1, MkWrA(0b11, 0b11, 0b11, 0b11), (v))
#define P1(v) v_f32_shuffle_b((v), lut_x1, 0, (v))
#define P2(v) v_f32_shuffle_b((v), lut_x2, 0, (v))
#define P4(v) v_f32_shuffle_b((v), lut_x4, 0, (v))
#if TOKENS == 16
#define SWAP_T(v) XOR16(v)
#else
#define SWAP_T(v) XOR8(v)
#endif
// all-true / all-false predicate from a scalar condition
#define PRED(cond) v_u32_cmp_less_b(lanes, (uint64)((cond) ? 64 : 0))
// v with lane (idx ^ x) moved to lane x for every x (idx < 64, runtime): lane 0 then holds v[idx]
#define LANE_TO_0(v, idx)                                                                                     \
    ({                                                                                                        \
        float64 _v = (v);                                                                                     \
        const int _i = (idx);                                                                                 \
        _v = v_f32_mov_vb(P1(_v), 0, _v, PRED(_i & 1));                                                        \
        _v = v_f32_mov_vb(P2(_v), 0, _v, PRED(_i & 2));                                                        \
        _v = v_f32_mov_vb(P4(_v), 0, _v, PRED(_i & 4));                                                        \
        _v = v_f32_mov_vb(XOR8(_v), 0, _v, PRED(_i & 8));                                                      \
        _v = v_f32_mov_vb(XOR16(_v), 0, _v, PRED(_i & 16));                                                    \
        _v = v_f32_mov_vb(XOR32(_v), 0, _v, PRED(_i & 32));                                                    \
        _v;                                                                                                   \
    })
#define LANE(v, idx) v_broadcast_element_f32(LANE_TO_0((v), (idx)), 0)

// LUT-free FP32 rsqrt(x) for x > 0 (3 Newton-Raphson steps -> full 23-bit FP32 precision, zero VLM LUT usage).
static inline float64 rsqrt_vec_f32(float64 x)
{
    float64 y = as_float64((int64)0x5f3759df - (as_int64(x) >> 1));
    const float64 h = 0.5f * x;
    y = y * (1.5f - h * y * y);
    y = y * (1.5f - h * y * y);
    y = y * (1.5f - h * y * y);
    return y;
}
static inline float64 exp_vec_f32(float64 x)
{
    const float64 xc = v_f32_max_b(x, -87.0f);
    const float64 t = xc * 1.4426950408889634f;
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.0096181291f;
    p = v_f32_mac_b(p, f, 0.0555041087f);
    p = v_f32_mac_b(p, f, 0.2402265070f);
    p = v_f32_mac_b(p, f, 0.6931471806f);
    p = v_f32_mac_b(p, f, 1.0f);
    const int64 bits = (n + 127) << 23;
    const float64 r = p * (*((float64*)&bits));
    return v_f32_sel_leq_f32_b(x, -87.0f, 0.0f, r);
}

// Per token t: lanes [0, T) -a_tj (0 for j >= t), [T, 2T) e_tj (0 for j > t), L_RKB.. per-token scalars.
volatile __local__ float64 token_coef[TOKENS];
// ds_t for rows 0..63 (index 2t) and 64..127 (index 2t + 1); zeroed once (masked terms multiply it by 0).
__local__ float64 token_delta[2 * TOKENS];
// VLM cache of gram rows k_t.k_j and q_t.k_j (2 * 16 * 256B = 8 KiB) to avoid loading gram twice!
__local__ float64 gram_k[TOKENS];
__local__ float64 gram_q[TOKENS];

void main(
#ifndef NO_PHASE2
          tensor source, tensor kvals,
#endif
          tensor log_decay, tensor beta, tensor rmat, tensor gram, tensor vals, tensor output
#ifndef NO_PHASE2
          , tensor checkpoints
#elif defined(OUTPUT_DS)
          , tensor ds_out
#endif
          )
{
    const uint64 lanes = read_lane_id_4b_b();
    const float q_scale = 0.08838834764831845f;  // 128 ** -0.5
    const int5 begin = get_index_space_offset();
    const int5 end = get_index_space_size() + begin;
    const float64 zero = 0.0f;
    for (int i = 0; i < 2 * TOKENS; ++i) token_delta[i] = zero;

    for (int batch = begin[3]; batch < end[3]; ++batch) {
        for (int head = begin[2]; head < end[2]; ++head) {
            const int key_head = head / 3;
            // ---- per-token scalars as lane vectors (lane t) ----
            // k_t.k_t is lane t of gram row t, q_t.q_t lane T + t of row T + t: select lanes, no movement
            float64 kdiag = zero, qdiag = zero, ldv = zero, betav = zero;
            int5 krow_coords = {0, 0, key_head, batch, 0};
            int5 qrow_coords = {0, TOKENS, key_head, batch, 0};
            float64 next_krow = v_f32_ld_tnsr_b(krow_coords, gram);
            float64 next_qrow = v_f32_ld_tnsr_b(qrow_coords, gram);
            for (int t = 0; t < TOKENS; ++t) {
                const float64 krow_now = next_krow, qrow_now = next_qrow;
                gram_k[t] = krow_now;
                gram_q[t] = qrow_now;
                const int tn = t + 1 < TOKENS ? t + 1 : t;
                krow_coords[1] = tn; qrow_coords[1] = TOKENS + tn;
                next_krow = v_f32_ld_tnsr_b(krow_coords, gram);
                next_qrow = v_f32_ld_tnsr_b(qrow_coords, gram);
                const bool64 here = v_u32_cmp_eq_b(lanes, (uint64)t);
                kdiag = v_f32_mov_vb(krow_now, 0, kdiag, here);
                qdiag = v_f32_mov_vb(qrow_now, 0, qdiag, v_u32_cmp_eq_b(lanes, (uint64)(TOKENS + t)));
                int5 scalar_coords = {head, t, batch, 0, 0};
                const float64 ld = v_broadcast_element_f32(v_f32_ld_tnsr_b(scalar_coords, log_decay), 0);
#ifdef BETA_F32
                // tensor-path load (as log_decay): no scalar global loads of data produced by the same graph
                const float64 bt = v_broadcast_element_f32(v_f32_ld_tnsr_b(scalar_coords, beta), 0);
                betav = v_f32_mov_vb(bt, 0, betav, here);
#else
                const float beta_value = (float)s_bf16_ld_g((__global__ bf16*)gen_addr(scalar_coords, beta));
                betav = v_f32_mov_vb(zero + beta_value, 0, betav, here);
#endif
                ldv = v_f32_mov_vb(ld, 0, ldv, here);
            }
            const float64 rkv = rsqrt_vec_f32(kdiag + 1e-6f);
            const float64 rqv = rsqrt_vec_f32(SWAP_T(qdiag) + 1e-6f) * q_scale;
            const float64 gv = exp_vec_f32(ldv);
            // G_t / G_j and G_t as running products of g (no exp per token: special-function LUT reloads made
            // the T16 kernel 4x slower; products also underflow gracefully where exp(cl_t - cl_j) would not)
            float64 ratio = zero;
            float64 Gcur = zero + 1.0f;
            // ---- coefficients (reads gram from 1-cycle VLM cache instead of reloading tensor) ----
            #pragma unroll
            for (int t = 0; t < TOKENS; ++t) {
                const float64 krow = gram_k[t];                                  // lanes j: k_t . k_j
                const float64 qrow = gram_q[t];                                  // lanes j: q_t . k_j
                const float64 g_t = v_broadcast_element_f32(gv, t);
                Gcur = Gcur * g_t;                                               // G_t
                ratio = ratio * g_t;                                             // lanes j < t: G_t / G_j
                ratio = v_f32_mov_vb(zero + 1.0f, 0, ratio, v_u32_cmp_eq_b(lanes, (uint64)t));
                const float64 rk_t = v_broadcast_element_f32(rkv, t);
                const float64 beta_t = v_broadcast_element_f32(betav, t);
                const float64 rq_t = v_broadcast_element_f32(rqv, t);
                const float64 neg_a = (ratio * krow) * (-(beta_t * rk_t * rk_t));
                const float64 e = (ratio * qrow) * rq_t;
                const float64 a_masked = v_f32_mov_vb(neg_a, 0, zero, v_u32_cmp_less_b(lanes, (uint64)t));
                const float64 e_masked = v_f32_mov_vb(e, 0, zero, v_u32_cmp_leq_b(lanes, (uint64)t));
                float64 coef = v_f32_mov_vb(SWAP_T(e_masked), 0, a_masked, v_u32_cmp_geq_b(lanes, (uint64)TOKENS));
                coef = v_f32_mov_vb(rk_t * beta_t, 0, coef, v_u32_cmp_eq_b(lanes, (uint64)L_RKB));
                coef = v_f32_mov_vb(beta_t * Gcur * rk_t * rk_t, 0, coef, v_u32_cmp_eq_b(lanes, (uint64)L_B));
                coef = v_f32_mov_vb(Gcur * rq_t, 0, coef, v_u32_cmp_eq_b(lanes, (uint64)L_C));
                coef = v_f32_mov_vb(g_t, 0, coef, v_u32_cmp_eq_b(lanes, (uint64)L_G));
                token_coef[t] = coef;
            }
            // ---- phase 1: ds_t and out_t for all rows (masked coefficients zero the j >= t / j > t terms) ----
#ifdef NO_PHASE2
            {
                int5 vc0 = {0, head, 0, batch, 0}, vc1 = {64, head, 0, batch, 0};
#ifdef RMAT_KH
                const int rbase = (head % 3) * 128;
                int5 kc0 = {rbase, 0, key_head, batch, 0}, kc1 = {rbase + 64, 0, key_head, batch, 0};
                int5 qc0 = {rbase, TOKENS, key_head, batch, 0}, qc1 = {rbase + 64, TOKENS, key_head, batch, 0};
#else
                int5 kc0 = {0, 0, head, batch, 0}, kc1 = {64, 0, head, batch, 0};
                int5 qc0 = {0, TOKENS, head, batch, 0}, qc1 = {64, TOKENS, head, batch, 0};
#endif
                float64 nv0 = v_f32_ld_tnsr_b(vc0, vals), nv1 = v_f32_ld_tnsr_b(vc1, vals);
                float64 nrk0 = v_f32_ld_tnsr_b(kc0, rmat), nrk1 = v_f32_ld_tnsr_b(kc1, rmat);
                float64 nrq0 = v_f32_ld_tnsr_b(qc0, rmat), nrq1 = v_f32_ld_tnsr_b(qc1, rmat);
                for (int t = 0; t < TOKENS; ++t) {
                    const float64 coef = token_coef[t];
                    const float64 v0 = nv0, v1 = nv1, rk0 = nrk0, rk1 = nrk1, rq0 = nrq0, rq1 = nrq1;
                    const int tn = t + 1 < TOKENS ? t + 1 : t;
                    vc0[2] = tn; vc1[2] = tn;
                    kc0[1] = tn; kc1[1] = tn;
                    qc0[1] = TOKENS + tn; qc1[1] = TOKENS + tn;
                    nv0 = v_f32_ld_tnsr_b(vc0, vals); nv1 = v_f32_ld_tnsr_b(vc1, vals);
                    nrk0 = v_f32_ld_tnsr_b(kc0, rmat); nrk1 = v_f32_ld_tnsr_b(kc1, rmat);
                    nrq0 = v_f32_ld_tnsr_b(qc0, rmat); nrq1 = v_f32_ld_tnsr_b(qc1, rmat);

                    const float64 c_rkb = v_broadcast_element_f32(coef, L_RKB);
                    const float64 c_b = v_broadcast_element_f32(coef, L_B);
                    float64 a0[4] = {v0 * c_rkb - rk0 * c_b, zero, zero, zero};
                    float64 a1[4] = {v1 * c_rkb - rk1 * c_b, zero, zero, zero};
                    if (t > 0) {
                        #pragma unroll
                        for (int j = 0; j < 4; ++j) {
                            const float64 cj = v_broadcast_element_f32(coef, j);
                            a0[j] = v_f32_mac_b(token_delta[2 * j], cj, a0[j]);
                            a1[j] = v_f32_mac_b(token_delta[2 * j + 1], cj, a1[j]);
                        }
                    }
                    if (t > 4) {
                        #pragma unroll
                        for (int j = 4; j < 8; ++j) {
                            const float64 cj = v_broadcast_element_f32(coef, j);
                            a0[j & 3] = v_f32_mac_b(token_delta[2 * j], cj, a0[j & 3]);
                            a1[j & 3] = v_f32_mac_b(token_delta[2 * j + 1], cj, a1[j & 3]);
                        }
                    }
#if TOKENS == 16
                    if (t > 8) {
                        #pragma unroll
                        for (int j = 8; j < 12; ++j) {
                            const float64 cj = v_broadcast_element_f32(coef, j);
                            a0[j & 3] = v_f32_mac_b(token_delta[2 * j], cj, a0[j & 3]);
                            a1[j & 3] = v_f32_mac_b(token_delta[2 * j + 1], cj, a1[j & 3]);
                        }
                    }
                    if (t > 12) {
                        #pragma unroll
                        for (int j = 12; j < 16; ++j) {
                            const float64 cj = v_broadcast_element_f32(coef, j);
                            a0[j & 3] = v_f32_mac_b(token_delta[2 * j], cj, a0[j & 3]);
                            a1[j & 3] = v_f32_mac_b(token_delta[2 * j + 1], cj, a1[j & 3]);
                        }
                    }
#endif
                    const float64 ds0 = (a0[0] + a0[1]) + (a0[2] + a0[3]);
                    const float64 ds1 = (a1[0] + a1[1]) + (a1[2] + a1[3]);
                    token_delta[2 * t] = ds0;
                    token_delta[2 * t + 1] = ds1;
#ifdef OUTPUT_DS
                    {
                        int5 dc0 = {0, head, t, batch, 0}, dc1 = {64, head, t, batch, 0};
                        v_f32_st_tnsr(dc0, ds_out, ds0);
                        v_f32_st_tnsr(dc1, ds_out, ds1);
                    }
#endif
                    const float64 c_c = v_broadcast_element_f32(coef, L_C);
                    float64 o0[4] = {rq0 * c_c, zero, zero, zero};
                    float64 o1[4] = {rq1 * c_c, zero, zero, zero};
                    #pragma unroll
                    for (int j = 0; j < 4; ++j) {
                        const float64 ej = v_broadcast_element_f32(coef, TOKENS + j);
                        o0[j] = v_f32_mac_b(token_delta[2 * j], ej, o0[j]);
                        o1[j] = v_f32_mac_b(token_delta[2 * j + 1], ej, o1[j]);
                    }
                    if (t >= 4) {
                        #pragma unroll
                        for (int j = 4; j < 8; ++j) {
                            const float64 ej = v_broadcast_element_f32(coef, TOKENS + j);
                            o0[j & 3] = v_f32_mac_b(token_delta[2 * j], ej, o0[j & 3]);
                            o1[j & 3] = v_f32_mac_b(token_delta[2 * j + 1], ej, o1[j & 3]);
                        }
                    }
#if TOKENS == 16
                    if (t >= 8) {
                        #pragma unroll
                        for (int j = 8; j < 12; ++j) {
                            const float64 ej = v_broadcast_element_f32(coef, TOKENS + j);
                            o0[j & 3] = v_f32_mac_b(token_delta[2 * j], ej, o0[j & 3]);
                            o1[j & 3] = v_f32_mac_b(token_delta[2 * j + 1], ej, o1[j & 3]);
                        }
                    }
                    if (t >= 12) {
                        #pragma unroll
                        for (int j = 12; j < 16; ++j) {
                            const float64 ej = v_broadcast_element_f32(coef, TOKENS + j);
                            o0[j & 3] = v_f32_mac_b(token_delta[2 * j], ej, o0[j & 3]);
                            o1[j & 3] = v_f32_mac_b(token_delta[2 * j + 1], ej, o1[j & 3]);
                        }
                    }
#endif
                    int5 oc0 = {0, head, t, batch, 0}, oc1 = {64, head, t, batch, 0};
                    v_f32_st_tnsr(oc0, output, (o0[0] + o0[1]) + (o0[2] + o0[3]));
                    v_f32_st_tnsr(oc1, output, (o1[0] + o1[1]) + (o1[2] + o1[3]));
                }
            }
#else
            #pragma unroll
            for (int part = 0; part < 2; ++part) {
                // software pipelined: the next token's global loads are issued before this token's chain
                int5 vcoords = {64 * part, head, 0, batch, 0};
#ifdef RMAT_KH
                // rmat [B, 16, 2T, 3 * 128]: the three value heads of a key head side by side
                int5 kcoords1 = {(head % 3) * 128 + 64 * part, 0, key_head, batch, 0};
                int5 qcoords1 = {(head % 3) * 128 + 64 * part, TOKENS, key_head, batch, 0};
#else
                int5 kcoords1 = {64 * part, 0, head, batch, 0};
                int5 qcoords1 = {64 * part, TOKENS, head, batch, 0};
#endif
                float64 next_values = v_f32_ld_tnsr_b(vcoords, vals);
                float64 next_rk = v_f32_ld_tnsr_b(kcoords1, rmat);
                float64 next_rq = v_f32_ld_tnsr_b(qcoords1, rmat);
                for (int t = 0; t < TOKENS; ++t) {
                    const float64 coef = token_coef[t];
                    const float64 values = next_values, rk_col = next_rk, rq_now = next_rq;
                    const int tn = t + 1 < TOKENS ? t + 1 : t;
                    vcoords[2] = tn; kcoords1[1] = tn; qcoords1[1] = TOKENS + tn;
                    next_values = v_f32_ld_tnsr_b(vcoords, vals);
                    next_rk = v_f32_ld_tnsr_b(kcoords1, rmat);
                    next_rq = v_f32_ld_tnsr_b(qcoords1, rmat);
                    float64 acc[4];
                    acc[0] = values * v_broadcast_element_f32(coef, L_RKB) - rk_col * v_broadcast_element_f32(coef, L_B);
                    acc[1] = zero; acc[2] = zero; acc[3] = zero;
                    #pragma unroll
                    for (int j = 0; j < TOKENS; ++j)
                        acc[j & 3] = v_f32_mac_b(token_delta[2 * j + part], v_broadcast_element_f32(coef, j), acc[j & 3]);
                    const float64 ds = (acc[0] + acc[1]) + (acc[2] + acc[3]);
                    token_delta[2 * t + part] = ds;
#ifdef OUTPUT_DS
                    {
                        int5 dc = {64 * part, head, t, batch, 0};
                        v_f32_st_tnsr(dc, ds_out, ds);
                    }
#endif
                    const float64 rq_col = rq_now;
                    float64 oc[4];
                    oc[0] = rq_col * v_broadcast_element_f32(coef, L_C);
                    oc[1] = zero; oc[2] = zero; oc[3] = zero;
                    #pragma unroll
                    for (int j = 0; j < TOKENS; ++j)
                        oc[j & 3] = v_f32_mac_b(token_delta[2 * j + part], v_broadcast_element_f32(coef, TOKENS + j),
                                                oc[j & 3]);
                    const float64 o = (oc[0] + oc[1]) + (oc[2] + oc[3]);
                    int5 out_coords = {64 * part, head, t, batch, 0};
                    v_f32_st_tnsr(out_coords, output, o);
                }
            }
#endif
#if defined(DEFER) && TOKENS == 16
            // ---- deferred: no state update here; ds_t goes to checkpoints[b, t, h, 0, :] (row 0 of each
            // checkpoint), the caller rebuilds the accepted state from S0 and these deltas in the next round ----
            for (int t = 0; t < TOKENS; ++t) {
                #pragma unroll
                for (int part = 0; part < 2; ++part) {
                    int5 coords = {64 * part, 0, head, t, batch};
                    v_f32_st_tnsr(coords, checkpoints, token_delta[2 * t + part]);
                }
            }
#elif !defined(NO_PHASE2)
            // ---- phase 2: state update and checkpoints, tiles of ROWS rows ----
            for (int tile = 0; tile < 128 / ROWS; ++tile) {
                const int part = tile >> 3;
                const int sub = tile & 7;                          // tile rows are lanes 8 sub .. 8 sub + 7
                const bool64 p8 = PRED(sub & 1), p16 = PRED(sub & 2), p32 = PRED(sub & 4);
                float64 state_lo[ROWS];
                float64 state_hi[ROWS];
                #pragma unroll
                for (int row = 0; row < ROWS; ++row) {
                    int5 coords = {0, tile * ROWS + row, head, batch, 0};
                    state_lo[row] = v_f32_ld_tnsr_b(coords, source);
                    coords[0] = 64;
                    state_hi[row] = v_f32_ld_tnsr_b(coords, source);
                }
#ifdef REPLAY_TOKENS
                for (int t = 0; t < REPLAY_TOKENS; ++t) {
#else
                for (int t = 0; t < TOKENS; ++t) {
#endif
                    float64 dv = token_delta[2 * t + part];
                    dv = v_f32_mov_vb(XOR8(dv), 0, dv, p8);
                    dv = v_f32_mov_vb(XOR16(dv), 0, dv, p16);
                    dv = v_f32_mov_vb(XOR32(dv), 0, dv, p32);       // lanes 0..7: ds_t of the tile rows
                    const float64 g = v_broadcast_element_f32(token_coef[t], L_G);
                    int5 kcoords = {0, key_head, t, batch, 0};
                    const float64 k_lo = v_f32_ld_tnsr_b(kcoords, kvals);
                    kcoords[0] = 64;
                    const float64 k_hi = v_f32_ld_tnsr_b(kcoords, kvals);
                    #pragma unroll
                    for (int row = 0; row < ROWS; ++row) {
                        const float64 dr = v_broadcast_element_f32(dv, row);
                        state_lo[row] = v_f32_mac_b(k_lo, dr, state_lo[row] * g);
                        state_hi[row] = v_f32_mac_b(k_hi, dr, state_hi[row] * g);
#ifdef STORE_LAST
#ifdef REPLAY_TOKENS
                        if (t == REPLAY_TOKENS - 1) {
#else
                        if (t == TOKENS - 1) {
#endif
#endif
                        int5 coords = {0, tile * ROWS + row, head, t, batch};
                        v_f32_st_tnsr(coords, checkpoints, state_lo[row]);
                        coords[0] = 64;
                        v_f32_st_tnsr(coords, checkpoints, state_hi[row]);
#ifdef STORE_LAST
                        }
#endif
                    }
                }
            }
#endif
        }
    }
}

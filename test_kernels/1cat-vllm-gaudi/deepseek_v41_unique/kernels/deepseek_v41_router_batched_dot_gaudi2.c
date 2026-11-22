// SPDX-License-Identifier: Apache-2.0
// C2-C6 dense Router: coalesced BF16 activation loads, FP32 weight reuse.
// Broadcast each activation in VPU registers; no scalar global reads in K.
#pragma clang fp contract(off)
#define ROWS(F) F(0) F(1) F(2) F(3) F(4) F(5)
void main(tensor input, tensor weights_kn, tensor partials) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int rows=get_dim_size(input,1);
    for(int part=begin[1];part<end[1];++part) {
        for(int block=begin[0];block<end[0];++block) {
#define ZERO(R) float64 acc##R=0;
            ROWS(ZERO)
#undef ZERO
            for(int tile=part*1280;tile<(part+1)*1280;tile+=128) {
#define LOAD(R) const float128 x##R=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b((int5){tile,R},input,0,(bfloat128)0,rows>R),SW_LINEAR);
                ROWS(LOAD)
#undef LOAD
#define COPY(R) const float64 b##R=v_f32_mov_dual_group_all_b(x##R.HALF,0xffffffff,DG,DG,DG,DG,MkWrA(3,3,3,3),0);
#define DOT(R) { const float64 coefficient=v_f32_shuffle_b(b##R,(uchar256)(0x80|(lane&7)|((lane&8)<<2)),0,b##R); acc##R=v_f32_mac_b(w,coefficient,acc##R); }
#define GROUP_BODY(H,D,O) { ROWS(COPY) for(int lane=0;lane<16;++lane) { const float64 w=v_f32_ld_tnsr_b((int5){block*64,tile+O+lane},weights_kn); ROWS(DOT) } }
#define HALF v1
#define DG 0
                GROUP_BODY(v1,0,0)
#undef DG
#define DG 1
                GROUP_BODY(v1,1,16)
#undef DG
#define DG 2
                GROUP_BODY(v1,2,32)
#undef DG
#define DG 3
                GROUP_BODY(v1,3,48)
#undef DG
#undef HALF
#define HALF v2
#define DG 0
                GROUP_BODY(v2,0,64)
#undef DG
#define DG 1
                GROUP_BODY(v2,1,80)
#undef DG
#define DG 2
                GROUP_BODY(v2,2,96)
#undef DG
#define DG 3
                GROUP_BODY(v2,3,112)
#undef DG
#undef HALF
#undef GROUP_BODY
#undef COPY
#undef DOT
            }
#define STORE(R) if(rows>R) v_f32_st_tnsr((int5){block*64,R,part},partials,acc##R);
            ROWS(STORE)
#undef STORE
        }
    }
}

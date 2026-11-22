// SPDX-License-Identifier: Apache-2.0
// The load-time bank has the exact existing decoder's [E,K,N] BF16 layout.
// Each routed output owns one N128/K32 rectangle, ready for the MME slicer.
void main(tensor ids,tensor bank,tensor output) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    const int experts=get_dim_size(bank,2);
    for(int slot=start[2];slot<end[2];++slot) {
        const int expert=s_i32_ld_g(gen_addr((int5){slot,0,0,0,0},ids));
        for(int kb=start[1];kb<end[1];++kb) {
            for(int nb=start[0];nb<end[0];++nb) {
                int5 src={nb*128,kb*32,expert,0,0},dst={nb*128,kb*32,slot,0,0};
                for(int row=0;row<32;++row) {
                    bfloat128 value={0};
                    if(expert>=0 && expert<experts)value=v_bf16_ld_tnsr_b(src,bank);
                    v_bf16_st_tnsr(dst,output,value);
                    ++src[1];++dst[1];
                }
            }
        }
    }
}

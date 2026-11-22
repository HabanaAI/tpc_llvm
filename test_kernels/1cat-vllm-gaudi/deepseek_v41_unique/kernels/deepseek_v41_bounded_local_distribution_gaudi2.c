// SPDX-License-Identifier: Apache-2.0
void main(tensor normalized,tensor ids,tensor output,int columns,int offset) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int row=begin[0];row<end[0];++row) {
        for(int col=0;col<columns;col+=64)
            v_f32_st_tnsr_partial((int5){col,row},output,(float64)0,
                                 columns-col<64?columns-col-1:63,0);
        // The vector zero-fill and scalar retained writes share one output.
        // Complete the former before publishing any nonzero probability.
        aso(SW_INC|SW_VPU);aso(SW_DEC|SW_SPU);
        for(int lane=0;lane<64;++lane) {
            const int index=s_i32_ld_g(gen_addr((int5){lane,row},ids))-offset;
            if(index>=0 && index<columns) {
                const float probability=s_f32_ld_g(gen_addr((int5){lane,row},normalized));
                s_f32_st_g(gen_addr((int5){index,row},output),probability);
            }
        }
    }
}

// SPDX-License-Identifier: Apache-2.0
// Keep the draw's scratch binding alive through the final public outputs.
void main(tensor result,tensor fine_flags,tensor token,tensor covered) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int row=begin[0];row<end[0];++row) {
        const int id=s_i32_ld_g(gen_addr((int5){0,row},result));
        const int valid=s_i32_ld_g(gen_addr((int5){1,row},result));
        const int columns=get_dim_size(fine_flags,0);
        int64 lowest=0;
        for(int col=0;col<columns;col+=64)
            lowest=v_i32_min_b(lowest,v_i32_ld_tnsr_partial_b((int5){col,row},fine_flags,
                columns-col<64 ? columns-col-1 : 63,0));
        lowest=v_i32_reduce_min(lowest);
        // Keep the certificate vector-valued to the final store. A tensor
        // output must not be read via gen_addr before its store completes.
        s_i32_st_g(gen_addr((int5){row},token),id);
        const int64 accepted=v_i32_sel_geq_i32_b(lowest,0,(int64)valid,(int64)0);
        v_i32_st_tnsr_partial((int5){row},covered,accepted,0,0);
    }
}

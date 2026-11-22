// SPDX-License-Identifier: Apache-2.0
// Restore owner rows and add the six BF16 W2 results in original route order.
void main(tensor matrices,tensor metadata,tensor output) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    for(int token=start[1];token<end[1];++token) {
        int owners[6],rows[6];
        for(int route=0;route<6;++route) {
            const int address=s_i32_ld_g(gen_addr((int5){token*6+route+9,0,0,0,0},metadata));
            owners[route]=-1;rows[route]=0;
            if(address<0) continue;
            const bool tail=(address&(1<<20))!=0;
            const int destination=(address>>9)&2047;
            owners[route]=tail?destination/6:destination/36;
            rows[route]=tail?destination%6:destination%36;
        }
        for(int chunk=start[0];chunk<end[0];++chunk) {
            float128 accumulated={0};
            for(int route=0;route<6;++route) {
                bfloat128 value=0;
                if(owners[route]>=0)
                    value=v_bf16_ld_tnsr_b((int5){chunk*128,rows[route],owners[route],0,0},matrices);
                const float128 widened=v_convert_bf16_to_f32_all_b(value);
                if(route==0) accumulated=widened;
                else {accumulated.v1+=widened.v1;accumulated.v2+=widened.v2;}
            }
            v_bf16_st_tnsr((int5){chunk*128,token,0,0,0},output,
                v_convert_f32_to_bf16_all_b(accumulated,SW_RHNE));
        }
    }
}

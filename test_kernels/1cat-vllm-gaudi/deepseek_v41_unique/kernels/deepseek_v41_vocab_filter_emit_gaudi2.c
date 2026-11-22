// SPDX-License-Identifier: Apache-2.0
// Read the compact bitmap and only retained scores; overflow is certified bad.
void main(tensor scores,tensor masks,tensor values,tensor ids,tensor counts,int columns,int width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int row=begin[0];row<end[0];++row) {
        for(int first=0;first<width;first+=64) {
            v_f32_st_tnsr_partial((int5){first,row},values,(float64)(-1.0f/0.0f),
                                 width-first<64?width-first-1:63,0);
            v_i32_st_tnsr_partial((int5){first,row},ids,(int64)0,
                                 width-first<64?width-first-1:63,0);
        }
        int written=0;
        for(int word=0;word<(columns+31)/32 && written<=width;++word) {
            unsigned active=s_u32_ld_g(gen_addr((int5){word,row},masks));
            while(active && written<=width) {
                const int index=word*32+s_u32_find_first(active,SW_FIND_ONE|SW_LSB);
                if(written<width) {
                    s_f32_st_g(gen_addr((int5){written,row},values),s_f32_ld_g(gen_addr((int5){index,row},scores)));
                    s_i32_st_g(gen_addr((int5){written,row},ids),index);
                }
                ++written;active&=active-1;
            }
        }
        s_i32_st_g(gen_addr((int5){0,row},counts),written);
    }
}

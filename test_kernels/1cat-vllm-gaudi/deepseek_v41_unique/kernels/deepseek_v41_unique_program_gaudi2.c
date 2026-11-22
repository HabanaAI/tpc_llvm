// SPDX-License-Identifier: Apache-2.0
// Select the exact M1..6 program for the owner's full count.  V5 selected a
// modulo-6 tail because it also emitted full M6 blocks; V6 owns one matrix.
void main(tensor bank, tensor spans, tensor metadata, tensor output) {
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    for(int blob=begin[0];blob<end[0];++blob) {
        const int first=s_i32_ld_g(gen_addr((int5){0,blob,0,0,0},spans));
        const int words=s_i32_ld_g(gen_addr((int5){1,blob,0,0,0},spans));
        if(words<=0) continue;
        const int owner=s_i32_ld_g(gen_addr((int5){2,blob,0,0,0},spans));
        int selected=5;
        if(owner>=0) {
            const int rows=s_i32_ld_g(gen_addr((int5){owner,3,0,0,0},metadata));
            selected=s_i32_min(s_i32_max(rows,1),6)-1;
        }
        for(int offset=0;offset<words;offset+=64) {
            const int64 value=v_i32_ld_tnsr_b((int5){first+offset,selected,0,0,0},bank);
            const int count=s_i32_min(words-offset,64);
            v_i32_st_tnsr_partial((int5){first+offset,0,0,0,0},output,value,count-1,0);
        }
    }
}

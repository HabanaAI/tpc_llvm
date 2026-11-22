// SPDX-License-Identifier: Apache-2.0
// The zeroed map is internal to this compound, retained through completion.
void main(tensor selection,tensor positions,tensor lengths,tensor map,tensor completion) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int capacity=get_dim_size(map,0)-256;
    for(int token=begin[1];token<end[1];++token) {
        const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
        const int position=s_i32_ld_g(gen_addr((int5){token},positions));
        for(int slot=begin[0];slot<end[0];++slot) {
            int key=-1;
            if(slot<length) {
                if(slot<128) {
                    const int absolute=position-127+slot;
                    if(absolute>=0)key=absolute&255;
                } else {
                    const int logical=s_i32_ld_g(gen_addr((int5){slot-128,token},selection));
                    if(logical>=0)key=logical<capacity?256+logical:-2;
                }
            }
            if(key>=0)s_i32_st_g(gen_addr((int5){key,token},map),slot+1);
            s_i32_st_g(gen_addr((int5){slot,token},completion),key);
        }
    }
}

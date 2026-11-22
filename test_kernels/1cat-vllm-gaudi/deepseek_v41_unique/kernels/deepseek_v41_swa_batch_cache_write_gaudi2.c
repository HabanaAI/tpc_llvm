// SPDX-License-Identifier: Apache-2.0
#define DSV41_DECODED_KV_WRITE 1
#include "deepseek_v41_swa_pack.h"
void main(tensor cache, tensor value, tensor positions, tensor decoded, tensor completion) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int token=begin[1]; token<end[1]; ++token) {
        const int logical=s_i32_ld_g(gen_addr((int5){token},positions));
        const int row=logical&255;
        for(int group=begin[0]; group<end[0]; ++group) {
            if(logical>=0)swa_pack_group(value,cache,group,token,row,decoded,row);
            s_i32_st_g(gen_addr((int5){group,token},completion),logical>=0?row:-1);
        }
    }
}

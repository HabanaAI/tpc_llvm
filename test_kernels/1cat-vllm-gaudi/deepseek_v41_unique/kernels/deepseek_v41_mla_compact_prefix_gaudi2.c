// SPDX-License-Identifier: Apache-2.0
void main(tensor counts,tensor prefix) {
    const int buckets=get_dim_size(counts,0);
    int offset=0;
    for(int bucket=0;bucket<buckets;++bucket) {
        s_i32_st_g(gen_addr((int5){bucket},prefix),offset);
        offset+=s_i32_ld_g(gen_addr((int5){bucket},counts));
    }
    s_i32_st_g(gen_addr((int5){buckets},prefix),offset);
}

// SPDX-License-Identifier: Apache-2.0
#pragma once
#define DSV41_MLA_HASH_BUCKETS 4352
static inline int mla_bucket(int logical) { return 256+((logical^(logical>>12))&4095); }
static inline int mla_source(tensor selection,int token,int slot) {
    return s_i32_ld_g(gen_addr((int5){slot-128,token},selection));
}

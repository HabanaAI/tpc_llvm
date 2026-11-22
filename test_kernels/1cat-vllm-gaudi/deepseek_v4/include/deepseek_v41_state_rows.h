// SPDX-License-Identifier: Apache-2.0
// Request-row mutation/read with an explicit producer completion. Negative
// rows represent padding and cannot modify either live or reserved state.
#if defined(DSV41_ROWS_U8)
#define LANES 256
#define VECTOR uchar256
#define LOAD v_u8_ld_tnsr_b
#define STORE v_u8_st_tnsr_partial
#elif defined(DSV41_ROWS_BF16)
#define LANES 128
#define VECTOR bfloat128
#define LOAD v_bf16_ld_tnsr_b
#define STORE v_bf16_st_tnsr_partial
#else
#define LANES 64
#define VECTOR float64
#define LOAD v_f32_ld_tnsr_b
#define STORE v_f32_st_tnsr_partial
#endif
#ifdef DSV41_ROWS_READ
void main(tensor cache, tensor rows, tensor completion, tensor output) {
#else
void main(tensor cache, tensor value, tensor rows, tensor completion) {
#endif
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    const int width=get_dim_size(cache,0), count=get_dim_size(cache,1);
    for(int batch=begin[0];batch<end[0];++batch) {
        const int row=s_i32_ld_g(gen_addr((int5){batch,0,0,0,0},rows));
        const bool valid=row>=0 && row<count;
#ifdef DSV41_ROWS_READ
        // The token is a data dependency, not a host-side readiness query.
        const int done=s_i32_ld_g(gen_addr((int5){batch,0,0,0,0},completion));
        for(int column=0;column<width;column+=LANES) {
            const int last=s_i32_min(LANES,width-column)-1;
            VECTOR v=0;
            if(valid && done>=0) v=LOAD((int5){column,row,0,0,0},cache);
            STORE((int5){column,batch,0,0,0},output,v,last,0);
        }
#else
        if(valid) for(int column=0;column<width;column+=LANES) {
            const int last=s_i32_min(LANES,width-column)-1;
            const VECTOR v=LOAD((int5){column,batch,0,0,0},value);
            STORE((int5){column,row,0,0,0},cache,v,last,0);
        }
        s_i32_st_g(gen_addr((int5){batch,0,0,0,0},completion),valid ? row : -1);
#endif
    }
}

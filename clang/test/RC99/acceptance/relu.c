// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o -
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o -

// batch norm for floats
void main(tensor ifm , tensor ofm)
{
    int5 addr=  0; 
    float64 zeros = 0;
    bool256 pp = 1;
    bool256 p = 0;
    p = bv_mov_b_b(1,p,1,0);
    // spatial for loops
    for (int h = 0 ; h < 10; h++)
    {
        addr[1] = h;
        for (int w = 0 ; w < 10; w++)
        {
            addr[2] = w;
            float64 tmp = 0;
            tmp = v_f32_ld_tnsr_i_b(addr,ifm,tmp,1,0);
            tmp = v_f32_max_v_v_vb(tmp,zeros,tmp,p,0);
            f32_st_tnsr_i_v_b(addr,ofm,tmp,1,0);
        }
    }
}

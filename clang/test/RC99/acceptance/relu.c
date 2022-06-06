// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o -
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o -

// batch norm for floats
void main(tensor ifm , tensor ofm)
{
    int5 addr=  0; 
    float64 zeros = 0;
    bool256 pp = 1;
    bool256 p = 0;
    p = v_i1_mov_i1_b(1, 0, p, 1, 0);
    // spatial for loops
    for (int h = 0 ; h < 10; h++)
    {
        addr[1] = h;
        for (int w = 0 ; w < 10; w++)
        {
            addr[2] = w;
            float64 tmp = 0;
            tmp = v_f32_ld_tnsr_b(addr, ifm, 0, tmp, 1, 0);
            tmp = v_f32_max_vb(tmp, zeros, 0, tmp, to_bool64(p), 0);
            v_f32_st_tnsr(addr, ofm, tmp, 0, 1, 0);
        }
    }
}

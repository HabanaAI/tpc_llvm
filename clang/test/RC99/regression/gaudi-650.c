// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o -

void main(float src_v1, float src_v2,
            tensor input_tnsr, tensor out_tnsr)
{
    int5 out_index = {-1,0,0,0,0};

    float64_float64_pair_t result_float64_pair;

    float64_float64_pair_t f64_pair_src = { src_v1, src_v2 };
    //float64_float64_pair_t f64_pair_src = { 1.0, 2.0 }; // no carsh

    result_float64_pair = v_f32_sel2_grt_f32_b  // comment out - no crash
    (
        (float64)1.0,
        (float64)1.0,
        (float64)1.0,
        (float64)1.0,
        0,
        f64_pair_src,
        1,
        0
    );

    out_index[0] += 1;
    v_f32_st_tnsr(out_index, out_tnsr, result_float64_pair.v1, 0, 1, 0); // comment out - no crash
}


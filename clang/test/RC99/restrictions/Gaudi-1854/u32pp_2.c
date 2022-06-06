// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-hwwa-ash-zero-scale=1 %s -o - | FileCheck %s

void main(tensor data1, tensor data2, tensor scale1, tensor scale2, tensor out1, tensor out2)
{
    int5 data_index = {0,0,0,0,0};

    uint64 data_1   = v_u32_ld_tnsr_b(data_index, data1);
    uint64 data_2   = v_u32_ld_tnsr_b(data_index, data2);
    char256 scale_1 = v_i8_ld_tnsr_b(data_index,  scale1);
    char256 scale_2 = v_i8_ld_tnsr_b(data_index,  scale2);

    uint128 res1 = {0};
    res1  = v_u32_mul_b(data_1, 1, SW_KEEP_RS, res1, 1, 0);

    uint64 res2 = 0;
    res2  = v_u32_ash_rhaz_b(res1, scale_1, 1, 0);

    uint128 res3 = {0};
    res3  = v_u32_mul_b(data_2, 1, SW_KEEP_RS, res3, 1, 0);

    uint64 res4 = 0;
    res4  = v_u32_ash_rhaz_b(res3, scale_2, 1, 0);

    int5 out_index = {0,0,0,0,0};
    v_u32_st_tnsr(out_index, out1, res2, 0, 1, 0);

    v_u32_st_tnsr(out_index, out2, res4, 0, 1, 0);
}

// CHECK-DAG: ld_tnsr  [[ORIG1:%V[0-9]+]], 0x2, %I2
// CHECK-DAG: mul.u32  keep_rs_for_add {{%D[0-9]+}}, {{%V[0-9]+}}, 0x1
// CHECK-DAG: add.u32  [[DEST1:%V[0-9]+]], {{%V[0-9]+}}, {{%V[0-9]+}}
// CHECK-DAG: and.u32  [[SHIFT1:%V[0-9]+]], [[ORIG1]], 0xff
// CHECK-DAG: sel_eq.u32 {{%V[0-9]+}}, [[SHIFT1]], 0x0, [[DEST1]], {{%V[0-9]+}}

// CHECK-DAG: ld_tnsr  [[ORIG2:%V[0-9]+]], 0x3, %I2
// CHECK-DAG: mul.u32  keep_rs_for_add {{%D[0-9]+}}, {{%V[0-9]+}}, 0x1
// CHECK-DAG: add.u32  [[DEST2:%V[0-9]+]], {{%V[0-9]+}}, {{%V[0-9]+}}
// CHECK-DAG: and.u32  [[SHIFT2:%V[0-9]+]], [[ORIG2]], 0xff
// CHECK-DAG: sel_eq.u32 {{%V[0-9]+}}, [[SHIFT2]], 0x0, [[DEST2]], {{%V[0-9]+}}


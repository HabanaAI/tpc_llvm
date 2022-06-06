// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-hwwa-ash-zero-scale=1 %s -o - | FileCheck %s

void main(tensor data, tensor scale, tensor out)
{
    int5 data_index = {0,0,0,0,0};

    uint64 input1  =  v_u32_ld_tnsr_b(data_index, data);
    char256 input2 = v_i8_ld_tnsr_b(data_index, scale);

    uint128 res1 = {0};
    res1  = v_u32_mul_b(input1, 1, SW_KEEP_RS);

    uint64 res2 = 0;
    res2  = v_u32_ash_rhaz_b(res1, input2);

    int5 out_index = {0,0,0,0,0};
    v_u32_st_tnsr(out_index, out, res2, 0, 1, 0);
}

// CHECK:     ld_tnsr  [[ORIG:%V[0-9]+]], 0x1, %I2
// CHECK-DAG: mul.u32  keep_rs_for_add {{%D[0-9]+}}, {{%V[0-9]+}}, 0x1
// CHECK-DAG: add.u32  [[DEST:%V[0-9]+]], {{%V[0-9]+}}, {{%V[0-9]+}}
// CHECK-DAG: and.u32  [[SHIFT:%V[0-9]+]], [[ORIG]], 0xff
// CHECK-DAG: sel_eq.u32 {{%V[0-9]+}}, [[SHIFT]], 0x0, [[DEST]], {{%V[0-9]+}}


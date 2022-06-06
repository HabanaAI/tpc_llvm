//RUN: %tpc_clang -S %s -o - | FileCheck %s
//RUN: %tpc_clang -march=gaudi -S %s -o - | FileCheck %s

void main(tensor ofm)
{
    int64 v1 = 1;
    int64 v2 = 2;
    short128 out;
    out = v_convert_int32_to_i16_b(v1, 0, 0, SW_RD, out, 1, 0);
    out = v_convert_int32_to_i16_b(v2, 0, 1, SW_RD, out, 1, 0);

    int5 coords = {0, 0, 0, 0};
    v_i16_st_tnsr(coords, ofm, out, 0, 1, 0);
}

// CHECK: convert_int32 lane_sel=0 rd to_16 [[REG:%V[0-9]+]],
// CHECK: convert_int32 lane_sel=1 rd to_16 [[REG]]

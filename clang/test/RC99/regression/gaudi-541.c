//RUN: %tpc_clang -S %s -o - | FileCheck %s
//RUN: %tpc_clang -march=gaudi -S %s -o - | FileCheck %s

void main(tensor ofm)
{
    int64 v1 = 1;
    int64 v2 = 2;
    short128 out;
    out = v_convert_int32_to_i16_v_s(v1, 0, out, e_round_down, 0);
    out = v_convert_int32_to_i16_v_s(v2, 0, out, e_round_down, 1);

    int5 coords = {0, 0, 0, 0};
    i16_st_tnsr_i_v(coords, ofm, out);
}

// CHECK: convert_int32 lane_sel=0 rd to_16 [[REG:%V[0-9]+]],
// CHECK: convert_int32 lane_sel=1 rd to_16 [[REG]]

// RUN: %tpc_clang -march=gaudi2 -c -no-instr-compress %s -o %t.gen4.o
// RUN: %disasm --mcpu=gaudi2 %t.gen4.o | FileCheck %s

void main(tensor in_tnsr, tensor out_tnsr) 
{
    int5 out_index   = {0,0,0,0,0};
    uchar256 uchar256_var = 1;
    float256 float256_var = convert_uchar256_to_float256(uchar256_var, SW_RHNE);
    v_f32_st_tnsr (out_index, out_tnsr, float256_var.v1, 0, 1, 0);
}

//CHECK: nop; nop; convert.u8 all_lanes target_type=fp32 rhne V0, V0; nop

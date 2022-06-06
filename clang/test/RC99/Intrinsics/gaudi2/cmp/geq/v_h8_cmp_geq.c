// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

typedef _Bool bool;

void main(tensor in,tensor out,bool pred)
{
    int5 c0={0};
    int5 c1={1,0};
    bool256 d0=0,d1=1;
    minihalf256 v0=(minihalf)0;
    minihalf256 v1=(minihalf)1;
    uchar256 vu0,vu1;
    vu0 = v_u8_ld_tnsr_b(c0,out,0,0,1,0);
    vu1 = v_u8_ld_tnsr_b(c1,out,0,0,1,0);
    v0 = v_convert_u8_to_h8_b(vu0,0,v0,1,0);
    v1 = v_convert_u8_to_h8_b(vu1,0,v1,1,0);
    d0 = v_h8_cmp_geq_b(v0,v1, 0, d0, pred, 0);
    v_i1_st_tnsr(c0,out,d0,0,1,0);
    d1 = v_h8_cmp_geq_b(v0,(minihalf)8, 0, d0, pred, 0);
    d1 = v_h8_cmp_geq_vb(v0,v1, 0, d0, d1, 0); 
    v_i1_st_tnsr(c1,out,d1,0,1,0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: cmp_geq.f8_152 %VP{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP
//CHECK-ASM-DAG: cmp_geq.f8_152 %VP{{[0-9]+}}, %V{{[0-9]+}}, 0x48, %SP
//CHECK-ASM-DAG: cmp_geq.f8_152 %VP{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP

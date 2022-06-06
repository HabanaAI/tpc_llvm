// RUN: %codegen -emit-obj -cc1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 -O2 %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck --check-prefix=CHECK-ASM %s

#define X2 1<<6
void main(unsigned short x0, unsigned short step, int pred, int dest0, int dest1)
{
    unsigned char __local *res0 = (unsigned char  __local *)dest0;
    unsigned char __local *res1 = (unsigned char  __local *)dest1;
    uint32_t_pair_t temp_res0 = {0,0};
    uint32_t_pair_t temp_res1 = {0,0};
    temp_res0 = u32_udiv_4step(x0, 1 /*step*/, 0 /*sw*/,  temp_res0,  1, 0);
    *res0 = temp_res0.v1;
    temp_res1 = u32_udiv_4step(x0, step, 0 /*sw*/,  temp_res1, pred, 1);
    *res1 = temp_res1.v1;
    //X2
    temp_res0 = u32_udiv_4step(x0, 4 /*step*/, X2,  temp_res0,  1, 0);
    *res0 = temp_res0.v1;
    temp_res1 = u32_udiv_4step(x0, step, X2,  temp_res1, pred, 1);
    *res1 = temp_res1.v1;
}
//CHECK-ASM: main
//CHECK-ASM-DAG: udiv_4step.u32 0x1 Z{{[0-9]+}}, S{{[0-9]+}}
//CHECK-ASM-DAG: udiv_4step.u32 DIV_STEP Z{{[0-9]+}}, S{{[0-9]+}}, !SP{{[0-9]+}}
//CHECK-ASM-DAG: udiv_4step.u32 x2 0x4 Z{{[0-9]+}}, S{{[0-9]+}}
//CHECK-ASM-DAG: udiv_4step.u32 x2 DIV_STEP Z{{[0-9]+}}, S{{[0-9]+}}, !SP{{[0-9]+}}
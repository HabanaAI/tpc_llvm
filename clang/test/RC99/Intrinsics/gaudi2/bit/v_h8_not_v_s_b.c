// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, minihalf x1, int x2, int dest0, int dest1)
{
    
    minihalf256 __local *ptr_x0 = (minihalf256 __local *)x0;
    
   minihalf256 __local *res0 = (minihalf256  __local *)dest0;
    minihalf256 temp_res0 = 0;
    temp_res0 = v_h8_not_b( (minihalf)8, 0,temp_res0, x2, 0);
    *res0 = temp_res0;
    
    minihalf256 __local *res1 = (minihalf256  __local *)dest1;
    minihalf256 temp_res1 = 0;
    temp_res1 = v_h8_not_b( x1, 0,temp_res1, x2, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: not.f8_152 %V{{[0-9]+}}, %V{{[0-9]+}},  %SP{{[0-9]+}}
//CHECK-ASM-DAG: not.f8_152 %V{{[0-9]+}}, %V{{[0-9]+}},  %SP{{[0-9]+}}

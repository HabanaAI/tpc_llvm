// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    
    minihalf256 __local *ptr_x0 = (minihalf256 __local *)x0;
    minihalf256 __local *ptr_x1 = (minihalf256 __local *)x1;
    
    minihalf256 __local *res0 = (minihalf256  __local *)dest0;
    minihalf256 temp_res0 = 0;
    temp_res0 = v_h8_not_b( *ptr_x1,0,temp_res0,1,0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: not.f8_152 %V{{[0-9]+}}, %V{{[0-9]+}}

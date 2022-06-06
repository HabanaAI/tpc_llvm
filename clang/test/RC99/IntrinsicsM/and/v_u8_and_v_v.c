// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    
    uchar256 __local *ptr_x0 = (uchar256 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    
   uchar256 __local *res0 = (uchar256  __local *)dest0;
    uchar256 temp_res0 = 0;
    temp_res0 = v_u8_and_b(*ptr_x0, *ptr_x1, 0, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.u8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int x2, int dest0)
{
    
    short128 __local *ptr_x0 = (short128 __local *)x0;
    short128 __local *ptr_x1 = (short128 __local *)x1;
    
   short128 __local *res0 = (short128  __local *)dest0;
    short128 temp_res0 = 0;
    temp_res0 = v_i16_or_b(*ptr_x0, *ptr_x1, 0, temp_res0, x2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: or.i16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

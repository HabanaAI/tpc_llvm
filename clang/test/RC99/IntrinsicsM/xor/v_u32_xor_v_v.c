// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    
    uint64 __local *ptr_x0 = (uint64 __local *)x0;
    uint64 __local *ptr_x1 = (uint64 __local *)x1;
    
   uint64 __local *res0 = (uint64  __local *)dest0;
    uint64 temp_res0 = 0;
    temp_res0 = v_u32_xor_b(*ptr_x0, *ptr_x1, 0, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.u32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

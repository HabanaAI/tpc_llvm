// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int x2, int dest0, int dest1)
{
    
    int64 __local *ptr_x0 = (int64 __local *)x0;
    
   int64 __local *res0 = (int64  __local *)dest0;
    int64 temp_res0 = 0;
    temp_res0 = v_i32_xor_b(*ptr_x0, 1, 0, temp_res0, x2, 0);
    *res0 = temp_res0;
    
   int64 __local *res1 = (int64  __local *)dest1;
    int64 temp_res1 = 0;
    temp_res1 = v_i32_xor_b(*ptr_x0, x1, 0, temp_res1, x2, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %SP{{[0-9]+}}
//CHECK-ASM-DAG: xor.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

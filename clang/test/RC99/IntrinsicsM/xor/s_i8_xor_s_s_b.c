// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(signed char x0, signed char x1, int x2, int dest0, int dest1)
{
    
    
    
   signed char __local *res0 = (signed char  __local *)dest0;
    signed char temp_res0 = 0;
    temp_res0 = s_i8_xor_s_s_b(x0, x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    
   signed char __local *res1 = (signed char  __local *)dest1;
    signed char temp_res1 = 0;
    temp_res1 = s_i8_xor_s_s_b(x0, 1, temp_res1, x2, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.i8 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
//CHECK-ASM-DAG: xor.i8 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1, %SP{{[0-9]+}}

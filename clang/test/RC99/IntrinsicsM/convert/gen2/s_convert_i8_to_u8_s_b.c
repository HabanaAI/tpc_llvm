// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(signed char x0, int x1, int dest0)
{
    
    
    
   unsigned char __local *res0 = (unsigned char  __local *)dest0;
    unsigned char temp_res0 = 0;
    temp_res0 = s_convert_i8_to_u8_s_b(x0, temp_res0, x1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.i8 target_type=uint8 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(signed char x0, int dest0)
{
    
    
    
   unsigned __local *res0 = (unsigned  __local *)dest0;
    unsigned temp_res0 = 0;
    temp_res0 = s_convert_i8_to_u32_s(x0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.i8 target_type=uint32 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0

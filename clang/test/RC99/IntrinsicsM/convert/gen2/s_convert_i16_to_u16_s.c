// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(short x0, int dest0)
{
    
    
    
   unsigned short __local *res0 = (unsigned short  __local *)dest0;
    unsigned short temp_res0 = 0;
    temp_res0 = s_convert_i16_to_u16_s(x0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.i16 target_type=uint16 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0

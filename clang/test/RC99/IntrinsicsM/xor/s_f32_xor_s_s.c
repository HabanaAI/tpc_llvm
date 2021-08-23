// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(float x0, float x1, int dest0, int dest1)
{
    
    
    
   float __local *res0 = (float  __local *)dest0;
    float temp_res0 = 0;
    temp_res0 = s_f32_xor_s_s(x0, x1);
    *res0 = temp_res0;
    
   float __local *res1 = (float  __local *)dest1;
    float temp_res1 = 0;
    temp_res1 = s_f32_xor_s_s(x0, 8.);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.f32 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: xor.f32 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x41000000

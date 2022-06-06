// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minihalf x0, int x1, int dest0)
{
   minihalf __local *res0 = (minihalf  __local *)dest0;
    unsigned char temp_res0 = 0;
    temp_res0 = s_h8_brev(x0, 0, temp_res0, x1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: brev.f8_152 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

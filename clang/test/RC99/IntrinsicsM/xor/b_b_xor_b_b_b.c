// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int x2, int dest0)
{
    
    
    _Bool res0 = 0; 
int __local *pred_res0 = (int  __local *)dest0;

    res0 = s_i1_xor(x0, x1, 0, res0, x2, 0);
    *pred_res0 = s_i32_add(*pred_res0, 1, 1, *pred_res0, res0, 0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.b %SP{{[0-9]+}}, %SP{{[0-9]+}}, %SP{{[0-9]+}}, %SP{{[0-9]+}}

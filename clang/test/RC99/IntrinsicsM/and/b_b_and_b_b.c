// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    
    
    _Bool res0 = 0; 
int __local *pred_res0 = (int  __local *)dest0;

    res0 = b_b_and_b_b(x0, x1);
    *pred_res0 = s_i32_add_s_s_b(*pred_res0, 1, *pred_res0, 1, res0, 0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.b %SP{{[0-9]+}}, %SP{{[0-9]+}}, %SP{{[0-9]+}}

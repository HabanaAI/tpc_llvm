// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O2 %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck --check-prefix=CHECK-O1 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s

// batch norm for floats
void main(tensor ifm , tensor mean_tensor, tensor var_tensor)
{
    int5 addr=  0; 
    float64 mean = 0;
    float64 var = 0;

    // spatial for loops
    for (int h = 0 ; h < 10; h++)
    {
        addr[1] = h;
        for (int w = 0 ; w < 10; w++)
        {
            addr[2] = w;
            float64 tmp = 0;
            tmp = v_f32_ld_tnsr_b(addr, ifm, 0, tmp, 1, 0);
            mean += tmp;
            var += tmp * tmp;
        }
    }
    int5 storeCoord = 0;
    v_f32_st_tnsr(storeCoord, mean_tensor, mean, 0, 1, 0);
    v_f32_st_tnsr(storeCoord, var_tensor, var, 0, 1, 0);
}

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt

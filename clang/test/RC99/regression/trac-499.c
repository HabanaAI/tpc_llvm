// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O2 %s -o - |  FileCheck %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - |  FileCheck --check-prefix=CHECK-O1 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s

// GAUDI-126
// AIL: *

 __local__ float ss;

void main(tensor ifm , tensor mean_tensor, tensor var_tensor)
{

    int5 a = { 1 ,2 ,3 ,4,5};
    float64 mean = a[0];
    ss = 0.56;
    int5 storeCoord = {0,0,0,0,0};
    for (int z = 0 ; z < a[1]; z += 1)
    {
        v_f32_st_tnsr(storeCoord, mean_tensor, z==0 ? ss : mean, 0, 1, 0);
    }
}

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt

// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt

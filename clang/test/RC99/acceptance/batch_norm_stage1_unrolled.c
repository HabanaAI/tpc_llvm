// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O2 %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck --check-prefix=CHECK-O1 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s

// batch norm for floats
void main(tensor ifm , tensor mean_tensor, tensor var_tensor)
{
    int5 addr0=  0; 
    int5 addr1=  0; 
    int5 addr2=  0; 
    int5 addr3=  0; 

    float64 mean = 0;
    float64 var = 0;

    // spatial for loops
    for (int h = 0 ; h < 20; h +=4)
    {
        addr0[1] = h;
        addr1[1] = h;
        addr2[1] = h;
        addr3[1] = h;
        for (int w = 0 ; w < 20; w+=4)
        {
            addr0[2] = w;
            addr1[2] = w+1;
            addr2[2] = w+2;
            addr3[2] = w+3;
            float64 tmp0, tmp1,tmp2,tmp3;
            tmp0 = v_f32_ld_tnsr_i_b(addr0,ifm,tmp0,1,0);
            tmp1 = v_f32_ld_tnsr_i_b(addr1,ifm,tmp1,1,0);
            tmp2 = v_f32_ld_tnsr_i_b(addr2,ifm,tmp2,1,0);
            tmp3 = v_f32_ld_tnsr_i_b(addr3,ifm,tmp3,1,0);
            
            mean += tmp0;
            var += tmp0 * tmp0;
            //nop
            // nop
            mean += tmp1;
            var += tmp1 * tmp1;
            //nop
            // nop
            mean += tmp2;
            var += tmp2 * tmp2;
            // nop
            // nop
            mean += tmp3;
            var += tmp3 * tmp3;
        }
    }
    int5 storeCoord = 0;
    f32_st_tnsr_i_v_b(storeCoord, mean_tensor, mean, 1,0);
    f32_st_tnsr_i_v_b(storeCoord, var_tensor, var, 1,0);
}

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt

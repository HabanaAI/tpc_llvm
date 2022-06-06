// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O2 %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck --check-prefix=CHECK-O1 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s
    
const int c_latencyCount = 3;
const int half_kw = 1;
const int half_kh = 1;
const int kernel_width = 3;
const int kernel_height = 3;
const int padw = 1;
const int padh = 1;


void main(tensor ifm, tensor filter, tensor ofm)         
{    
    int5 index_space_size = { 256 ,12 ,12 ,1,1};
    int256 accum;    
    // iterate over IFM depth
    for (int d = 0 ; d <  index_space_size[0]; d += 256)
    {
        //iterate over image width
        for (int w = 0 ; w < index_space_size[1]; w += 1)
        {
            //load a column of H values for for the right most W value of the kernel
            for (int h = 0 ; h <  index_space_size[2] ; h++)
            {
                accum.v1 = 0;
                accum.v2 = 0;
                accum.v3 = 0;
                accum.v4 = 0;
                //iterate over kernel 
                for (int kh = 0 ; kh < kernel_height; kh++)
                {
                    for (int kw = 0 ; kw < kernel_width; kw++)
                    {
                        int5 ifmIndex = { d,
                                         w + kw - padw,
                                         h + kh - padh,
                                         0 ,0} ;  
                        char256 ifmValue = 0;
                        ifmValue = v_i8_ld_tnsr_b(ifmIndex, ifm, 0, ifmValue, 1, 0);
                        int5 filterIndex = {d,kw,kh,0,0};
                        char256 filterValue = 0;
                        ifmValue = v_i8_ld_tnsr_b(filterIndex, ifm, 0, ifmValue, 1, 0);
                        accum = v_i8_mac_b(ifmValue, filterValue, accum, 1/*saturated*/, 1, 0);
                    }
                }

                int5 ofmIndex = {d,w,h,0,0};
                v_i32_st_tnsr(ofmIndex, ofm, accum.v1, 0, 1, 0);
                ofmIndex[0] = d+64;
                v_i32_st_tnsr(ofmIndex, ofm, accum.v2, 0, 1, 0);
                ofmIndex[0] = d+128;
                v_i32_st_tnsr(ofmIndex, ofm, accum.v3, 0, 1, 0);
                ofmIndex[0] = d+192;
                v_i32_st_tnsr(ofmIndex, ofm, accum.v4, 0, 1, 0);
            }
        }
    }    
}

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt

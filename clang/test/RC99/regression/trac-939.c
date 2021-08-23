// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s

// TODO check convert encoding
// CHECK: main

// CHECK-ASM: main
// CHECK-ASM: halt

// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt

void main(tensor ifm,
          tensor ofm,
          tensor debug_tensor,
            int mul_factor_4,
            int shift_factor_4,
            int mul_factor_6,
            int shift_factor_6,
            int mul_factor_9,
            int shift_factor_9)

{
    int5 index_space_size = get_index_space_size();
    int5 output_coords = {0};
               
    for (int b = 0 ; b <  index_space_size[3]; b += 1)
    {
        output_coords[3] = b;
        for (int h = 0 ; h <  index_space_size[2]; h += 1)
        {
            output_coords[2] = h;
            for (int w = 0 ; w <  index_space_size[1]; w += 1)
            {
                output_coords[1] = w;
                for (int d = 0 ; d <  index_space_size[0]; d += 256)
                {
                   
                    output_coords[0] = d;
   
                    int256 accum =  {0};    
                    char mul_factor = mul_factor_6;
                    char shift_factor = shift_factor_6;
                    // iterate over filter width/height
                   
                    //calculate multiplication and shift factors once per inner kernel
                     char low_edge_w = (w == 0);
                     char high_edge_w = (w == index_space_size[1]-1);
                     char low_edge_h = (h == 0);
                     char high_edge_h = (h == index_space_size[2]-1);
                     char middle_w = (!low_edge_w && !high_edge_w);
                     char middle_h = (!low_edge_h && !high_edge_h);
                           
                     if (middle_h && middle_w) {
                       mul_factor = mul_factor_9;
                       shift_factor = shift_factor_9;
                     }
                           
                     if ((low_edge_w || high_edge_w) && (low_edge_h || high_edge_h)){
                       mul_factor = mul_factor_4;
                       shift_factor = shift_factor_4;
                     }
                    
                    
                    for (int kw = w - 1 ; kw <= w + 1; kw++)
                    {
                        for (int kh = h - 1 ; kh <= h + 1; kh++)
                        {
                           
                            int5 ifmIndex = { d, kw, kh, b, 0 } ; 
                            char256 ifmValue = v_i8_ld_tnsr_i_b(ifmIndex, ifm, 0 /*source*/, 1, 1);
                         
                            accum = av_i8_mac_v_s_b(ifmValue, mul_factor, accum, 1/*saturated*/, 1, 0);
                        }
                    }
 
                    char256 accum_out;
                    accum_out = v_convert_int32_to_i8_v_s_b(accum.v1, -shift_factor, 0 /*source*/, 0 /*RNE*/, 0, 1 /*don't predicate */, 1);
                    accum_out = v_convert_int32_to_i8_v_s_b(accum.v2, -shift_factor, 0 /*source*/, 0 /*RNE*/, 1, 1 /*don't predicate */, 1);
                    accum_out = v_convert_int32_to_i8_v_s_b(accum.v3, -shift_factor, 0 /*source*/, 0 /*RNE*/, 2, 1 /*don't predicate */, 1);
                    accum_out = v_convert_int32_to_i8_v_s_b(accum.v4, -shift_factor, 0 /*source*/, 0 /*RNE*/, 3, 1 /*don't predicate */, 1);
 
                    i8_st_tnsr_i_v_b(output_coords,ofm,accum_out,1,0);
                   
                } //iteration over OFM depth
            } //iteration over OFM height
        } //iteration over OFM width
    } // iteration over batch
}


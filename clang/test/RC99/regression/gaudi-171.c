// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o -
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o -
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O0 %s -o -

void main(tensor ifm0,
          tensor ifm1,
          tensor ifm2,
          tensor ifm3,
          tensor ofm0,
          tensor ofm1,
          tensor ofm2,
          tensor ofm3,
          char padw,
          char padh)
{
    int5 index_space_start = get_index_space_offset();
    int5 index_space_end = get_index_space_size() + index_space_start;

    int5 ifmIndex0;
    int5 ifmIndex1;
    int5 ifmIndex2;
    int5 ifmIndex3;
    
    int5 ofmIndex0;
    int5 ofmIndex1;
    int5 ofmIndex2;
    int5 ofmIndex3;

    for (int d = index_space_start[0]*256 ; d < index_space_end[0]*256; d += 256)
    {
        ifmIndex0[0] = d;
        ifmIndex1[0] = d;
        ifmIndex2[0] = d;
        ifmIndex3[0] = d;
        ofmIndex0[0] = d;
        ofmIndex1[0] = d;
        ofmIndex2[0] = d;
        ofmIndex3[0] = d;
        for (int b = index_space_start[3] ; b < index_space_end[3]; b += 1)
        {
            ifmIndex0[3] = b;
            ifmIndex1[3] = b;
            ifmIndex2[3] = b;
            ifmIndex3[3] = b;
            ofmIndex0[3] = b;
            ofmIndex1[3] = b;
            ofmIndex2[3] = b;
            ofmIndex3[3] = b;
            // processing window 6x3
            for (int h = index_space_start[2]*4 ; h < index_space_end[2]*4 + 3 - padh; h += 4)
            {
                ifmIndex0[1] = index_space_start[1] - padw;
                ifmIndex1[1] = index_space_start[1] - padw;
                ifmIndex2[1] = index_space_start[1] - padw;
                ifmIndex3[1] = index_space_start[1] - padw;
                ofmIndex0[1] = index_space_start[1];
                ofmIndex1[1] = index_space_start[1];
                ofmIndex2[1] = index_space_start[1];
                ofmIndex3[1] = index_space_start[1];
                ifmIndex0[2] = h - padh;
                ifmIndex1[2] = h - padh;
                ifmIndex2[2] = h - padh;
                ifmIndex3[2] = h - padh;
                ofmIndex0[2] = h;
                ofmIndex1[2] = h;
                ofmIndex2[2] = h;
                ofmIndex3[2] = h;

                // column 0
                char256 ifmValue00 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                char256 ifmValue01 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                char256 ifmValue02 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                char256 ifmValue03 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue10 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                char256 ifmValue11 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                char256 ifmValue12 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                char256 ifmValue13 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue20 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                char256 ifmValue21 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                char256 ifmValue22 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                char256 ifmValue23 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue30 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                char256 ifmValue31 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                char256 ifmValue32 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                char256 ifmValue33 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue40 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                char256 ifmValue41 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                char256 ifmValue42 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                char256 ifmValue43 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue50 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                char256 ifmValue51 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                char256 ifmValue52 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                char256 ifmValue53 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] -= 5;
                ifmIndex1[2] -= 5;
                ifmIndex2[2] -= 5;
                ifmIndex3[2] -= 5;
                ifmIndex0[1] += 1;
                ifmIndex1[1] += 1;
                ifmIndex2[1] += 1;
                ifmIndex3[1] += 1;

                char256 mid00 = v_i8_max_b(ifmValue10, ifmValue20, 0, 0, 1, 0);
                char256 mid01 = v_i8_max_b(ifmValue11, ifmValue21, 0, 0, 1, 0);
                char256 mid02 = v_i8_max_b(ifmValue12, ifmValue22, 0, 0, 1, 0);
                char256 mid03 = v_i8_max_b(ifmValue13, ifmValue23, 0, 0, 1, 0);
                
                char256 mid10 = v_i8_max_b(ifmValue30, ifmValue40, 0, 0, 1, 0);
                char256 mid11 = v_i8_max_b(ifmValue31, ifmValue41, 0, 0, 1, 0);
                char256 mid12 = v_i8_max_b(ifmValue32, ifmValue42, 0, 0, 1, 0);
                char256 mid13 = v_i8_max_b(ifmValue33, ifmValue43, 0, 0, 1, 0);

                char256 v00 = v_i8_max_b(ifmValue00, mid00, 0, 0, 1, 0);
                char256 v01 = v_i8_max_b(ifmValue01, mid01, 0, 0, 1, 0);
                char256 v02 = v_i8_max_b(ifmValue02, mid02, 0, 0, 1, 0);
                char256 v03 = v_i8_max_b(ifmValue03, mid03, 0, 0, 1, 0);
                
                char256 v10 = v_i8_max_b(mid00, ifmValue30, 0, 0, 1, 0);
                char256 v11 = v_i8_max_b(mid01, ifmValue31, 0, 0, 1, 0);
                char256 v12 = v_i8_max_b(mid02, ifmValue32, 0, 0, 1, 0);
                char256 v13 = v_i8_max_b(mid03, ifmValue33, 0, 0, 1, 0);
                
                char256 v20 = v_i8_max_b(ifmValue20, mid10, 0, 0, 1, 0);
                char256 v21 = v_i8_max_b(ifmValue21, mid11, 0, 0, 1, 0);
                char256 v22 = v_i8_max_b(ifmValue22, mid12, 0, 0, 1, 0);
                char256 v23 = v_i8_max_b(ifmValue23, mid13, 0, 0, 1, 0);
                
                char256 v30 = v_i8_max_b(mid10, ifmValue50, 0, 0, 1, 0);
                char256 v31 = v_i8_max_b(mid11, ifmValue51, 0, 0, 1, 0);
                char256 v32 = v_i8_max_b(mid12, ifmValue52, 0, 0, 1, 0);
                char256 v33 = v_i8_max_b(mid13, ifmValue53, 0, 0, 1, 0);

                // column 1
                ifmValue00 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue01 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue02 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue03 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue10 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue11 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue12 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue13 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue20 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue20 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue21 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue22 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue23 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue30 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue31 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue32 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue33 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue40 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue41 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue42 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue43 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue50 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue51 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue52 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue53 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] -= 5;
                ifmIndex1[2] -= 5;
                ifmIndex2[2] -= 5;
                ifmIndex3[2] -= 5;
                ifmIndex0[1] += 1;
                ifmIndex1[1] += 1;
                ifmIndex2[1] += 1;
                ifmIndex3[1] += 1;

                mid00 = v_i8_max_b(ifmValue10, ifmValue20, 0, 0, 1, 0);
                mid01 = v_i8_max_b(ifmValue11, ifmValue21, 0, 0, 1, 0);
                mid02 = v_i8_max_b(ifmValue12, ifmValue22, 0, 0, 1, 0);
                mid03 = v_i8_max_b(ifmValue13, ifmValue23, 0, 0, 1, 0);
                
                mid10 = v_i8_max_b(ifmValue30, ifmValue40, 0, 0, 1, 0);
                mid11 = v_i8_max_b(ifmValue31, ifmValue41, 0, 0, 1, 0);
                mid12 = v_i8_max_b(ifmValue32, ifmValue42, 0, 0, 1, 0);
                mid13 = v_i8_max_b(ifmValue33, ifmValue43, 0, 0, 1, 0);

                char256 v40 = v_i8_max_b(ifmValue00, mid00, 0, 0, 1, 0);
                char256 v41 = v_i8_max_b(ifmValue01, mid01, 0, 0, 1, 0);
                char256 v42 = v_i8_max_b(ifmValue02, mid02, 0, 0, 1, 0);
                char256 v43 = v_i8_max_b(ifmValue03, mid03, 0, 0, 1, 0);
                
                char256 v50 = v_i8_max_b(mid00, ifmValue30, 0, 0, 1, 0);
                char256 v51 = v_i8_max_b(mid01, ifmValue31, 0, 0, 1, 0);
                char256 v52 = v_i8_max_b(mid02, ifmValue32, 0, 0, 1, 0);
                char256 v53 = v_i8_max_b(mid03, ifmValue33, 0, 0, 1, 0);
                
                char256 v60 = v_i8_max_b(ifmValue20, mid10, 0, 0, 1, 0);
                char256 v61 = v_i8_max_b(ifmValue21, mid11, 0, 0, 1, 0);
                char256 v62 = v_i8_max_b(ifmValue22, mid12, 0, 0, 1, 0);
                char256 v63 = v_i8_max_b(ifmValue23, mid13, 0, 0, 1, 0);
                
                char256 v70 = v_i8_max_b(mid10, ifmValue50, 0, 0, 1, 0);
                char256 v71 = v_i8_max_b(mid11, ifmValue51, 0, 0, 1, 0);
                char256 v72 = v_i8_max_b(mid12, ifmValue52, 0, 0, 1, 0);
                char256 v73 = v_i8_max_b(mid13, ifmValue53, 0, 0, 1, 0);

                // column 2
                ifmValue00 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue01 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue02 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue03 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue10 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue11 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue12 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue13 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue20 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue21 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue22 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue23 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue30 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue31 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue32 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue33 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue40 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue41 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue42 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue43 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue50 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                ifmValue51 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                ifmValue52 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                ifmValue53 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                ifmIndex0[2] -= 5;
                ifmIndex1[2] -= 5;
                ifmIndex2[2] -= 5;
                ifmIndex3[2] -= 5;

                mid00 = v_i8_max_b(ifmValue10, ifmValue20, 0, 0, 1, 0);
                mid01 = v_i8_max_b(ifmValue11, ifmValue21, 0, 0, 1, 0);
                mid02 = v_i8_max_b(ifmValue12, ifmValue22, 0, 0, 1, 0);
                mid03 = v_i8_max_b(ifmValue13, ifmValue23, 0, 0, 1, 0);
                mid10 = v_i8_max_b(ifmValue30, ifmValue40, 0, 0, 1, 0);
                mid11 = v_i8_max_b(ifmValue31, ifmValue41, 0, 0, 1, 0);
                mid12 = v_i8_max_b(ifmValue32, ifmValue42, 0, 0, 1, 0);
                mid13 = v_i8_max_b(ifmValue33, ifmValue43, 0, 0, 1, 0);

                char256 v80 = v_i8_max_b(ifmValue00, mid00, 0, 0, 1, 0);
                char256 v81 = v_i8_max_b(ifmValue01, mid01, 0, 0, 1, 0);
                char256 v82 = v_i8_max_b(ifmValue02, mid02, 0, 0, 1, 0);
                char256 v83 = v_i8_max_b(ifmValue03, mid03, 0, 0, 1, 0);
                
                char256 v90 = v_i8_max_b(mid00, ifmValue30, 0, 0, 1, 0);
                char256 v91 = v_i8_max_b(mid01, ifmValue31, 0, 0, 1, 0);
                char256 v92 = v_i8_max_b(mid02, ifmValue32, 0, 0, 1, 0);
                char256 v93 = v_i8_max_b(mid03, ifmValue33, 0, 0, 1, 0);
                
                char256 va0 = v_i8_max_b(ifmValue20, mid10, 0, 0, 1, 0);
                char256 va1 = v_i8_max_b(ifmValue21, mid11, 0, 0, 1, 0);
                char256 va2 = v_i8_max_b(ifmValue22, mid12, 0, 0, 1, 0);
                char256 va3 = v_i8_max_b(ifmValue23, mid13, 0, 0, 1, 0);
                
                char256 vb0 = v_i8_max_b(mid10, ifmValue50, 0, 0, 1, 0);
                char256 vb1 = v_i8_max_b(mid11, ifmValue51, 0, 0, 1, 0);
                char256 vb2 = v_i8_max_b(mid12, ifmValue52, 0, 0, 1, 0);
                char256 vb3 = v_i8_max_b(mid13, ifmValue53, 0, 0, 1, 0);

                // extra 8 max ops to calculate 4 elements
                char256 v040 = v_i8_max_b(v00, v40, 0, 0, 1, 0);
                char256 v041 = v_i8_max_b(v01, v41, 0, 0, 1, 0);
                char256 v042 = v_i8_max_b(v02, v42, 0, 0, 1, 0);
                char256 v043 = v_i8_max_b(v03, v43, 0, 0, 1, 0);
                
                char256 v150 = v_i8_max_b(v10, v50, 0, 0, 1, 0);
                char256 v151 = v_i8_max_b(v11, v51, 0, 0, 1, 0);
                char256 v152 = v_i8_max_b(v12, v52, 0, 0, 1, 0);
                char256 v153 = v_i8_max_b(v13, v53, 0, 0, 1, 0);
                
                char256 v260 = v_i8_max_b(v20, v60, 0, 0, 1, 0);
                char256 v261 = v_i8_max_b(v21, v61, 0, 0, 1, 0);
                char256 v262 = v_i8_max_b(v22, v62, 0, 0, 1, 0);
                char256 v263 = v_i8_max_b(v23, v63, 0, 0, 1, 0);
                
                char256 v370 = v_i8_max_b(v30, v70, 0, 0, 1, 0);
                char256 v371 = v_i8_max_b(v31, v71, 0, 0, 1, 0);
                char256 v372 = v_i8_max_b(v32, v72, 0, 0, 1, 0);
                char256 v373 = v_i8_max_b(v33, v73, 0, 0, 1, 0);

                char256 m00 = v_i8_max_b(v040, v80, 0, 0, 1, 0);
                char256 m01 = v_i8_max_b(v041, v81, 0, 0, 1, 0);
                char256 m02 = v_i8_max_b(v042, v82, 0, 0, 1, 0);
                char256 m03 = v_i8_max_b(v043, v83, 0, 0, 1, 0);
                
                char256 m10 = v_i8_max_b(v150, v90, 0, 0, 1, 0);
                char256 m11 = v_i8_max_b(v151, v91, 0, 0, 1, 0);
                char256 m12 = v_i8_max_b(v152, v92, 0, 0, 1, 0);
                char256 m13 = v_i8_max_b(v153, v93, 0, 0, 1, 0);
                
                char256 m20 = v_i8_max_b(v260, va0, 0, 0, 1, 0);
                char256 m21 = v_i8_max_b(v261, va1, 0, 0, 1, 0);
                char256 m22 = v_i8_max_b(v262, va2, 0, 0, 1, 0);
                char256 m23 = v_i8_max_b(v263, va3, 0, 0, 1, 0);
                
                char256 m30 = v_i8_max_b(v370, vb0, 0, 0, 1, 0);
                char256 m31 = v_i8_max_b(v371, vb1, 0, 0, 1, 0);
                char256 m32 = v_i8_max_b(v372, vb2, 0, 0, 1, 0);
                char256 m33 = v_i8_max_b(v373, vb3, 0, 0, 1, 0);

                // storing first 4 elements
                v_i8_st_tnsr(ofmIndex0, ofm0, m00, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex1, ofm1, m01, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex2, ofm2, m02, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex3, ofm3, m03, 0, 1, 0);
                ofmIndex0[2] += 1;
                ofmIndex1[2] += 1;
                ofmIndex2[2] += 1;
                ofmIndex3[2] += 1;
                v_i8_st_tnsr(ofmIndex0, ofm0, m10, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex1, ofm1, m11, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex2, ofm2, m12, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex3, ofm3, m13, 0, 1, 0);
                ofmIndex0[2] += 1;
                ofmIndex1[2] += 1;
                ofmIndex2[2] += 1;
                ofmIndex3[2] += 1;
                v_i8_st_tnsr(ofmIndex0, ofm0, m20, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex1, ofm1, m21, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex2, ofm2, m22, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex3, ofm3, m23, 0, 1, 0);
                ofmIndex0[2] += 1;
                ofmIndex1[2] += 1;
                ofmIndex2[2] += 1;
                ofmIndex3[2] += 1;
                v_i8_st_tnsr(ofmIndex0, ofm0, m30, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex1, ofm1, m31, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex2, ofm2, m32, 0, 1, 0);
                v_i8_st_tnsr(ofmIndex3, ofm3, m33, 0, 1, 0);
                ofmIndex0[2] -= 3;
                ofmIndex1[2] -= 3;
                ofmIndex2[2] -= 3;
                ofmIndex3[2] -= 3;

                for (int w = index_space_start[1] + 1 ; w < index_space_end[1] + 3 - padw; w += 1)
                {
                    ifmIndex0[1] += 1;
                    ifmIndex1[1] += 1;
                    ifmIndex2[1] += 1;
                    ifmIndex3[1] += 1;
                    ofmIndex0[1] += 1;
                    ofmIndex1[1] += 1;
                    ofmIndex2[1] += 1;
                    ofmIndex3[1] += 1;

                    v00 = v40;
                    v01 = v41;
                    v02 = v42;
                    v03 = v43;
                    
                    v10 = v50;
                    v11 = v51;
                    v12 = v52;
                    v13 = v53;
                    
                    v20 = v60;
                    v21 = v61;
                    v22 = v62;
                    v23 = v63;
                    
                    v30 = v70;
                    v31 = v71;
                    v32 = v72;
                    v33 = v73;

                    v40 = v80;
                    v41 = v81;
                    v42 = v82;
                    v43 = v83;
                    
                    v50 = v90;
                    v51 = v91;
                    v52 = v92;
                    v53 = v93;
                    
                    v60 = va0;
                    v61 = va1;
                    v62 = va2;
                    v63 = va3;
                    
                    v70 = vb0;
                    v71 = vb1;
                    v72 = vb2;
                    v73 = vb3;

                    ifmValue00 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                    ifmValue01 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                    ifmValue02 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                    ifmValue03 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue10 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                    ifmValue11 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                    ifmValue12 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                    ifmValue13 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue20 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                    ifmValue21 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                    ifmValue22 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                    ifmValue23 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue30 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                    ifmValue31 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                    ifmValue32 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                    ifmValue33 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue40 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                    ifmValue41 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                    ifmValue42 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                    ifmValue43 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue50 = v_i8_ld_tnsr_b(ifmIndex0, ifm0, 0, 0, 1, 0);
                    ifmValue51 = v_i8_ld_tnsr_b(ifmIndex1, ifm1, 0, 0, 1, 0);
                    ifmValue52 = v_i8_ld_tnsr_b(ifmIndex2, ifm2, 0, 0, 1, 0);
                    ifmValue53 = v_i8_ld_tnsr_b(ifmIndex3, ifm3, 0, 0, 1, 0);
                    ifmIndex0[2] -= 5;
                    ifmIndex1[2] -= 5;
                    ifmIndex2[2] -= 5;
                    ifmIndex3[2] -= 5;

                    mid00 = v_i8_max_b(ifmValue10, ifmValue20, 0, 0, 1, 0);
                    mid01 = v_i8_max_b(ifmValue11, ifmValue21, 0, 0, 1, 0);
                    mid02 = v_i8_max_b(ifmValue12, ifmValue22, 0, 0, 1, 0);
                    mid03 = v_i8_max_b(ifmValue13, ifmValue23, 0, 0, 1, 0);
                    
                    mid10 = v_i8_max_b(ifmValue30, ifmValue40, 0, 0, 1, 0);
                    mid11 = v_i8_max_b(ifmValue31, ifmValue41, 0, 0, 1, 0);
                    mid12 = v_i8_max_b(ifmValue32, ifmValue42, 0, 0, 1, 0);
                    mid13 = v_i8_max_b(ifmValue33, ifmValue43, 0, 0, 1, 0);

                    v80 = v_i8_max_b(ifmValue00, mid00, 0, 0, 1, 0);
                    v81 = v_i8_max_b(ifmValue01, mid01, 0, 0, 1, 0);
                    v82 = v_i8_max_b(ifmValue02, mid02, 0, 0, 1, 0);
                    v83 = v_i8_max_b(ifmValue03, mid03, 0, 0, 1, 0);
                    
                    v90 = v_i8_max_b(mid00, ifmValue30, 0, 0, 1, 0);
                    v91 = v_i8_max_b(mid01, ifmValue31, 0, 0, 1, 0);
                    v92 = v_i8_max_b(mid02, ifmValue32, 0, 0, 1, 0);
                    v93 = v_i8_max_b(mid03, ifmValue33, 0, 0, 1, 0);
                    
                    va0 = v_i8_max_b(ifmValue20, mid10, 0, 0, 1, 0);
                    va1 = v_i8_max_b(ifmValue21, mid11, 0, 0, 1, 0);
                    va2 = v_i8_max_b(ifmValue22, mid12, 0, 0, 1, 0);
                    va3 = v_i8_max_b(ifmValue23, mid13, 0, 0, 1, 0);
                    
                    vb0 = v_i8_max_b(mid10, ifmValue50, 0, 0, 1, 0);
                    vb1 = v_i8_max_b(mid11, ifmValue51, 0, 0, 1, 0);
                    vb2 = v_i8_max_b(mid12, ifmValue52, 0, 0, 1, 0);
                    vb3 = v_i8_max_b(mid13, ifmValue53, 0, 0, 1, 0);

                    v040 = v_i8_max_b(v00, v40, 0, 0, 1, 0);
                    v041 = v_i8_max_b(v01, v41, 0, 0, 1, 0);
                    v042 = v_i8_max_b(v02, v42, 0, 0, 1, 0);
                    v043 = v_i8_max_b(v03, v43, 0, 0, 1, 0);
                    
                    v150 = v_i8_max_b(v10, v50, 0, 0, 1, 0);
                    v151 = v_i8_max_b(v11, v51, 0, 0, 1, 0);
                    v152 = v_i8_max_b(v12, v52, 0, 0, 1, 0);
                    v153 = v_i8_max_b(v13, v53, 0, 0, 1, 0);
                    
                    v260 = v_i8_max_b(v20, v60, 0, 0, 1, 0);
                    v261 = v_i8_max_b(v21, v61, 0, 0, 1, 0);
                    v262 = v_i8_max_b(v22, v62, 0, 0, 1, 0);
                    v263 = v_i8_max_b(v23, v63, 0, 0, 1, 0);
                    
                    v370 = v_i8_max_b(v30, v70, 0, 0, 1, 0);
                    v371 = v_i8_max_b(v31, v71, 0, 0, 1, 0);
                    v372 = v_i8_max_b(v32, v72, 0, 0, 1, 0);
                    v373 = v_i8_max_b(v33, v73, 0, 0, 1, 0);

                    m00 = v_i8_max_b(v040, v80, 0, 0, 1, 0);
                    m01 = v_i8_max_b(v041, v81, 0, 0, 1, 0);
                    m02 = v_i8_max_b(v042, v82, 0, 0, 1, 0);
                    m03 = v_i8_max_b(v043, v83, 0, 0, 1, 0);
                    
                    m10 = v_i8_max_b(v150, v90, 0, 0, 1, 0);
                    m11 = v_i8_max_b(v151, v91, 0, 0, 1, 0);
                    m12 = v_i8_max_b(v152, v92, 0, 0, 1, 0);
                    m13 = v_i8_max_b(v153, v93, 0, 0, 1, 0);
                    
                    m20 = v_i8_max_b(v260, va0, 0, 0, 1, 0);
                    m21 = v_i8_max_b(v261, va1, 0, 0, 1, 0);
                    m22 = v_i8_max_b(v262, va2, 0, 0, 1, 0);
                    m23 = v_i8_max_b(v263, va3, 0, 0, 1, 0);
                    
                    m30 = v_i8_max_b(v370, vb0, 0, 0, 1, 0);
                    m31 = v_i8_max_b(v371, vb1, 0, 0, 1, 0);
                    m32 = v_i8_max_b(v372, vb2, 0, 0, 1, 0);
                    m33 = v_i8_max_b(v373, vb3, 0, 0, 1, 0);

                    v_i8_st_tnsr(ofmIndex0, ofm0, m00, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex1, ofm1, m01, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex2, ofm2, m02, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex3, ofm3, m03, 0, 1, 0);
                    ofmIndex0[2] += 1;
                    ofmIndex1[2] += 1;
                    ofmIndex2[2] += 1;
                    ofmIndex3[2] += 1;
                    v_i8_st_tnsr(ofmIndex0, ofm0, m10, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex1, ofm1, m11, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex2, ofm2, m12, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex3, ofm3, m13, 0, 1, 0);
                    ofmIndex0[2] += 1;
                    ofmIndex1[2] += 1;
                    ofmIndex2[2] += 1;
                    ofmIndex3[2] += 1;
                    v_i8_st_tnsr(ofmIndex0, ofm0, m20, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex1, ofm1, m21, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex2, ofm2, m22, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex3, ofm3, m23, 0, 1, 0);
                    ofmIndex0[2] += 1;
                    ofmIndex1[2] += 1;
                    ofmIndex2[2] += 1;
                    ofmIndex3[2] += 1;
                    v_i8_st_tnsr(ofmIndex0, ofm0, m30, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex1, ofm1, m31, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex2, ofm2, m32, 0, 1, 0);
                    v_i8_st_tnsr(ofmIndex3, ofm3, m33, 0, 1, 0);
                    ofmIndex0[2] -= 3;
                    ofmIndex1[2] -= 3;
                    ofmIndex2[2] -= 3;
                    ofmIndex3[2] -= 3;
                }
            }
        }
    }
}

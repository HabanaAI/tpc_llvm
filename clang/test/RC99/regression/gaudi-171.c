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
                char256 ifmValue00 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                char256 ifmValue01 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                char256 ifmValue02 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                char256 ifmValue03 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue10 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                char256 ifmValue11 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                char256 ifmValue12 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                char256 ifmValue13 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue20 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                char256 ifmValue21 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                char256 ifmValue22 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                char256 ifmValue23 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue30 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                char256 ifmValue31 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                char256 ifmValue32 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                char256 ifmValue33 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue40 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                char256 ifmValue41 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                char256 ifmValue42 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                char256 ifmValue43 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                char256 ifmValue50 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                char256 ifmValue51 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                char256 ifmValue52 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                char256 ifmValue53 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] -= 5;
                ifmIndex1[2] -= 5;
                ifmIndex2[2] -= 5;
                ifmIndex3[2] -= 5;
                ifmIndex0[1] += 1;
                ifmIndex1[1] += 1;
                ifmIndex2[1] += 1;
                ifmIndex3[1] += 1;

                char256 mid00 = v_i8_max_v_v(ifmValue10, ifmValue20);
                char256 mid01 = v_i8_max_v_v(ifmValue11, ifmValue21);
                char256 mid02 = v_i8_max_v_v(ifmValue12, ifmValue22);
                char256 mid03 = v_i8_max_v_v(ifmValue13, ifmValue23);
                
                char256 mid10 = v_i8_max_v_v(ifmValue30, ifmValue40);
                char256 mid11 = v_i8_max_v_v(ifmValue31, ifmValue41);
                char256 mid12 = v_i8_max_v_v(ifmValue32, ifmValue42);
                char256 mid13 = v_i8_max_v_v(ifmValue33, ifmValue43);

                char256 v00 = v_i8_max_v_v(ifmValue00, mid00);
                char256 v01 = v_i8_max_v_v(ifmValue01, mid01);
                char256 v02 = v_i8_max_v_v(ifmValue02, mid02);
                char256 v03 = v_i8_max_v_v(ifmValue03, mid03);
                
                char256 v10 = v_i8_max_v_v(mid00, ifmValue30);
                char256 v11 = v_i8_max_v_v(mid01, ifmValue31);
                char256 v12 = v_i8_max_v_v(mid02, ifmValue32);
                char256 v13 = v_i8_max_v_v(mid03, ifmValue33);
                
                char256 v20 = v_i8_max_v_v(ifmValue20, mid10);
                char256 v21 = v_i8_max_v_v(ifmValue21, mid11);
                char256 v22 = v_i8_max_v_v(ifmValue22, mid12);
                char256 v23 = v_i8_max_v_v(ifmValue23, mid13);
                
                char256 v30 = v_i8_max_v_v(mid10, ifmValue50);
                char256 v31 = v_i8_max_v_v(mid11, ifmValue51);
                char256 v32 = v_i8_max_v_v(mid12, ifmValue52);
                char256 v33 = v_i8_max_v_v(mid13, ifmValue53);

                // column 1
                ifmValue00 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue01 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue02 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue03 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue10 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue11 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue12 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue13 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue20 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue20 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue21 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue22 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue23 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue30 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue31 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue32 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue33 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue40 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue41 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue42 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue43 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue50 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue51 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue52 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue53 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] -= 5;
                ifmIndex1[2] -= 5;
                ifmIndex2[2] -= 5;
                ifmIndex3[2] -= 5;
                ifmIndex0[1] += 1;
                ifmIndex1[1] += 1;
                ifmIndex2[1] += 1;
                ifmIndex3[1] += 1;

                mid00 = v_i8_max_v_v(ifmValue10, ifmValue20);
                mid01 = v_i8_max_v_v(ifmValue11, ifmValue21);
                mid02 = v_i8_max_v_v(ifmValue12, ifmValue22);
                mid03 = v_i8_max_v_v(ifmValue13, ifmValue23);
                
                mid10 = v_i8_max_v_v(ifmValue30, ifmValue40);
                mid11 = v_i8_max_v_v(ifmValue31, ifmValue41);
                mid12 = v_i8_max_v_v(ifmValue32, ifmValue42);
                mid13 = v_i8_max_v_v(ifmValue33, ifmValue43);

                char256 v40 = v_i8_max_v_v(ifmValue00, mid00);
                char256 v41 = v_i8_max_v_v(ifmValue01, mid01);
                char256 v42 = v_i8_max_v_v(ifmValue02, mid02);
                char256 v43 = v_i8_max_v_v(ifmValue03, mid03);
                
                char256 v50 = v_i8_max_v_v(mid00, ifmValue30);
                char256 v51 = v_i8_max_v_v(mid01, ifmValue31);
                char256 v52 = v_i8_max_v_v(mid02, ifmValue32);
                char256 v53 = v_i8_max_v_v(mid03, ifmValue33);
                
                char256 v60 = v_i8_max_v_v(ifmValue20, mid10);
                char256 v61 = v_i8_max_v_v(ifmValue21, mid11);
                char256 v62 = v_i8_max_v_v(ifmValue22, mid12);
                char256 v63 = v_i8_max_v_v(ifmValue23, mid13);
                
                char256 v70 = v_i8_max_v_v(mid10, ifmValue50);
                char256 v71 = v_i8_max_v_v(mid11, ifmValue51);
                char256 v72 = v_i8_max_v_v(mid12, ifmValue52);
                char256 v73 = v_i8_max_v_v(mid13, ifmValue53);

                // column 2
                ifmValue00 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue01 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue02 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue03 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue10 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue11 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue12 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue13 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue20 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue21 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue22 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue23 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue30 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue31 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue32 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue33 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue40 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue41 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue42 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue43 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] += 1;
                ifmIndex1[2] += 1;
                ifmIndex2[2] += 1;
                ifmIndex3[2] += 1;
                ifmValue50 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                ifmValue51 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                ifmValue52 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                ifmValue53 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                ifmIndex0[2] -= 5;
                ifmIndex1[2] -= 5;
                ifmIndex2[2] -= 5;
                ifmIndex3[2] -= 5;

                mid00 = v_i8_max_v_v(ifmValue10, ifmValue20);
                mid01 = v_i8_max_v_v(ifmValue11, ifmValue21);
                mid02 = v_i8_max_v_v(ifmValue12, ifmValue22);
                mid03 = v_i8_max_v_v(ifmValue13, ifmValue23);
                mid10 = v_i8_max_v_v(ifmValue30, ifmValue40);
                mid11 = v_i8_max_v_v(ifmValue31, ifmValue41);
                mid12 = v_i8_max_v_v(ifmValue32, ifmValue42);
                mid13 = v_i8_max_v_v(ifmValue33, ifmValue43);

                char256 v80 = v_i8_max_v_v(ifmValue00, mid00);
                char256 v81 = v_i8_max_v_v(ifmValue01, mid01);
                char256 v82 = v_i8_max_v_v(ifmValue02, mid02);
                char256 v83 = v_i8_max_v_v(ifmValue03, mid03);
                
                char256 v90 = v_i8_max_v_v(mid00, ifmValue30);
                char256 v91 = v_i8_max_v_v(mid01, ifmValue31);
                char256 v92 = v_i8_max_v_v(mid02, ifmValue32);
                char256 v93 = v_i8_max_v_v(mid03, ifmValue33);
                
                char256 va0 = v_i8_max_v_v(ifmValue20, mid10);
                char256 va1 = v_i8_max_v_v(ifmValue21, mid11);
                char256 va2 = v_i8_max_v_v(ifmValue22, mid12);
                char256 va3 = v_i8_max_v_v(ifmValue23, mid13);
                
                char256 vb0 = v_i8_max_v_v(mid10, ifmValue50);
                char256 vb1 = v_i8_max_v_v(mid11, ifmValue51);
                char256 vb2 = v_i8_max_v_v(mid12, ifmValue52);
                char256 vb3 = v_i8_max_v_v(mid13, ifmValue53);

                // extra 8 max ops to calculate 4 elements
                char256 v040 = v_i8_max_v_v(v00, v40);
                char256 v041 = v_i8_max_v_v(v01, v41);
                char256 v042 = v_i8_max_v_v(v02, v42);
                char256 v043 = v_i8_max_v_v(v03, v43);
                
                char256 v150 = v_i8_max_v_v(v10, v50);
                char256 v151 = v_i8_max_v_v(v11, v51);
                char256 v152 = v_i8_max_v_v(v12, v52);
                char256 v153 = v_i8_max_v_v(v13, v53);
                
                char256 v260 = v_i8_max_v_v(v20, v60);
                char256 v261 = v_i8_max_v_v(v21, v61);
                char256 v262 = v_i8_max_v_v(v22, v62);
                char256 v263 = v_i8_max_v_v(v23, v63);
                
                char256 v370 = v_i8_max_v_v(v30, v70);
                char256 v371 = v_i8_max_v_v(v31, v71);
                char256 v372 = v_i8_max_v_v(v32, v72);
                char256 v373 = v_i8_max_v_v(v33, v73);

                char256 m00 = v_i8_max_v_v(v040, v80);
                char256 m01 = v_i8_max_v_v(v041, v81);
                char256 m02 = v_i8_max_v_v(v042, v82);
                char256 m03 = v_i8_max_v_v(v043, v83);
                
                char256 m10 = v_i8_max_v_v(v150, v90);
                char256 m11 = v_i8_max_v_v(v151, v91);
                char256 m12 = v_i8_max_v_v(v152, v92);
                char256 m13 = v_i8_max_v_v(v153, v93);
                
                char256 m20 = v_i8_max_v_v(v260, va0);
                char256 m21 = v_i8_max_v_v(v261, va1);
                char256 m22 = v_i8_max_v_v(v262, va2);
                char256 m23 = v_i8_max_v_v(v263, va3);
                
                char256 m30 = v_i8_max_v_v(v370, vb0);
                char256 m31 = v_i8_max_v_v(v371, vb1);
                char256 m32 = v_i8_max_v_v(v372, vb2);
                char256 m33 = v_i8_max_v_v(v373, vb3);

                // storing first 4 elements
                i8_st_tnsr_i_v(ofmIndex0, ofm0, m00);
                i8_st_tnsr_i_v(ofmIndex1, ofm1, m01);
                i8_st_tnsr_i_v(ofmIndex2, ofm2, m02);
                i8_st_tnsr_i_v(ofmIndex3, ofm3, m03);
                ofmIndex0[2] += 1;
                ofmIndex1[2] += 1;
                ofmIndex2[2] += 1;
                ofmIndex3[2] += 1;
                i8_st_tnsr_i_v(ofmIndex0, ofm0, m10);
                i8_st_tnsr_i_v(ofmIndex1, ofm1, m11);
                i8_st_tnsr_i_v(ofmIndex2, ofm2, m12);
                i8_st_tnsr_i_v(ofmIndex3, ofm3, m13);
                ofmIndex0[2] += 1;
                ofmIndex1[2] += 1;
                ofmIndex2[2] += 1;
                ofmIndex3[2] += 1;
                i8_st_tnsr_i_v(ofmIndex0, ofm0, m20);
                i8_st_tnsr_i_v(ofmIndex1, ofm1, m21);
                i8_st_tnsr_i_v(ofmIndex2, ofm2, m22);
                i8_st_tnsr_i_v(ofmIndex3, ofm3, m23);
                ofmIndex0[2] += 1;
                ofmIndex1[2] += 1;
                ofmIndex2[2] += 1;
                ofmIndex3[2] += 1;
                i8_st_tnsr_i_v(ofmIndex0, ofm0, m30);
                i8_st_tnsr_i_v(ofmIndex1, ofm1, m31);
                i8_st_tnsr_i_v(ofmIndex2, ofm2, m32);
                i8_st_tnsr_i_v(ofmIndex3, ofm3, m33);
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

                    v00 = v_i8_mov_v(v40);
                    v01 = v_i8_mov_v(v41);
                    v02 = v_i8_mov_v(v42);
                    v03 = v_i8_mov_v(v43);
                    
                    v10 = v_i8_mov_v(v50);
                    v11 = v_i8_mov_v(v51);
                    v12 = v_i8_mov_v(v52);
                    v13 = v_i8_mov_v(v53);
                    
                    v20 = v_i8_mov_v(v60);
                    v21 = v_i8_mov_v(v61);
                    v22 = v_i8_mov_v(v62);
                    v23 = v_i8_mov_v(v63);
                    
                    v30 = v_i8_mov_v(v70);
                    v31 = v_i8_mov_v(v71);
                    v32 = v_i8_mov_v(v72);
                    v33 = v_i8_mov_v(v73);

                    v40 = v_i8_mov_v(v80);
                    v41 = v_i8_mov_v(v81);
                    v42 = v_i8_mov_v(v82);
                    v43 = v_i8_mov_v(v83);
                    
                    v50 = v_i8_mov_v(v90);
                    v51 = v_i8_mov_v(v91);
                    v52 = v_i8_mov_v(v92);
                    v53 = v_i8_mov_v(v93);
                    
                    v60 = v_i8_mov_v(va0);
                    v61 = v_i8_mov_v(va1);
                    v62 = v_i8_mov_v(va2);
                    v63 = v_i8_mov_v(va3);
                    
                    v70 = v_i8_mov_v(vb0);
                    v71 = v_i8_mov_v(vb1);
                    v72 = v_i8_mov_v(vb2);
                    v73 = v_i8_mov_v(vb3);

                    ifmValue00 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                    ifmValue01 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                    ifmValue02 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                    ifmValue03 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue10 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                    ifmValue11 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                    ifmValue12 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                    ifmValue13 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue20 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                    ifmValue21 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                    ifmValue22 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                    ifmValue23 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue30 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                    ifmValue31 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                    ifmValue32 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                    ifmValue33 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue40 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                    ifmValue41 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                    ifmValue42 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                    ifmValue43 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                    ifmIndex0[2] += 1;
                    ifmIndex1[2] += 1;
                    ifmIndex2[2] += 1;
                    ifmIndex3[2] += 1;
                    ifmValue50 = v_i8_ld_tnsr_i(ifmIndex0, ifm0);
                    ifmValue51 = v_i8_ld_tnsr_i(ifmIndex1, ifm1);
                    ifmValue52 = v_i8_ld_tnsr_i(ifmIndex2, ifm2);
                    ifmValue53 = v_i8_ld_tnsr_i(ifmIndex3, ifm3);
                    ifmIndex0[2] -= 5;
                    ifmIndex1[2] -= 5;
                    ifmIndex2[2] -= 5;
                    ifmIndex3[2] -= 5;

                    mid00 = v_i8_max_v_v(ifmValue10, ifmValue20);
                    mid01 = v_i8_max_v_v(ifmValue11, ifmValue21);
                    mid02 = v_i8_max_v_v(ifmValue12, ifmValue22);
                    mid03 = v_i8_max_v_v(ifmValue13, ifmValue23);
                    
                    mid10 = v_i8_max_v_v(ifmValue30, ifmValue40);
                    mid11 = v_i8_max_v_v(ifmValue31, ifmValue41);
                    mid12 = v_i8_max_v_v(ifmValue32, ifmValue42);
                    mid13 = v_i8_max_v_v(ifmValue33, ifmValue43);

                    v80 = v_i8_max_v_v(ifmValue00, mid00);
                    v81 = v_i8_max_v_v(ifmValue01, mid01);
                    v82 = v_i8_max_v_v(ifmValue02, mid02);
                    v83 = v_i8_max_v_v(ifmValue03, mid03);
                    
                    v90 = v_i8_max_v_v(mid00, ifmValue30);
                    v91 = v_i8_max_v_v(mid01, ifmValue31);
                    v92 = v_i8_max_v_v(mid02, ifmValue32);
                    v93 = v_i8_max_v_v(mid03, ifmValue33);
                    
                    va0 = v_i8_max_v_v(ifmValue20, mid10);
                    va1 = v_i8_max_v_v(ifmValue21, mid11);
                    va2 = v_i8_max_v_v(ifmValue22, mid12);
                    va3 = v_i8_max_v_v(ifmValue23, mid13);
                    
                    vb0 = v_i8_max_v_v(mid10, ifmValue50);
                    vb1 = v_i8_max_v_v(mid11, ifmValue51);
                    vb2 = v_i8_max_v_v(mid12, ifmValue52);
                    vb3 = v_i8_max_v_v(mid13, ifmValue53);

                    v040 = v_i8_max_v_v(v00, v40);
                    v041 = v_i8_max_v_v(v01, v41);
                    v042 = v_i8_max_v_v(v02, v42);
                    v043 = v_i8_max_v_v(v03, v43);
                    
                    v150 = v_i8_max_v_v(v10, v50);
                    v151 = v_i8_max_v_v(v11, v51);
                    v152 = v_i8_max_v_v(v12, v52);
                    v153 = v_i8_max_v_v(v13, v53);
                    
                    v260 = v_i8_max_v_v(v20, v60);
                    v261 = v_i8_max_v_v(v21, v61);
                    v262 = v_i8_max_v_v(v22, v62);
                    v263 = v_i8_max_v_v(v23, v63);
                    
                    v370 = v_i8_max_v_v(v30, v70);
                    v371 = v_i8_max_v_v(v31, v71);
                    v372 = v_i8_max_v_v(v32, v72);
                    v373 = v_i8_max_v_v(v33, v73);

                    m00 = v_i8_max_v_v(v040, v80);
                    m01 = v_i8_max_v_v(v041, v81);
                    m02 = v_i8_max_v_v(v042, v82);
                    m03 = v_i8_max_v_v(v043, v83);
                    
                    m10 = v_i8_max_v_v(v150, v90);
                    m11 = v_i8_max_v_v(v151, v91);
                    m12 = v_i8_max_v_v(v152, v92);
                    m13 = v_i8_max_v_v(v153, v93);
                    
                    m20 = v_i8_max_v_v(v260, va0);
                    m21 = v_i8_max_v_v(v261, va1);
                    m22 = v_i8_max_v_v(v262, va2);
                    m23 = v_i8_max_v_v(v263, va3);
                    
                    m30 = v_i8_max_v_v(v370, vb0);
                    m31 = v_i8_max_v_v(v371, vb1);
                    m32 = v_i8_max_v_v(v372, vb2);
                    m33 = v_i8_max_v_v(v373, vb3);

                    i8_st_tnsr_i_v(ofmIndex0, ofm0, m00);
                    i8_st_tnsr_i_v(ofmIndex1, ofm1, m01);
                    i8_st_tnsr_i_v(ofmIndex2, ofm2, m02);
                    i8_st_tnsr_i_v(ofmIndex3, ofm3, m03);
                    ofmIndex0[2] += 1;
                    ofmIndex1[2] += 1;
                    ofmIndex2[2] += 1;
                    ofmIndex3[2] += 1;
                    i8_st_tnsr_i_v(ofmIndex0, ofm0, m10);
                    i8_st_tnsr_i_v(ofmIndex1, ofm1, m11);
                    i8_st_tnsr_i_v(ofmIndex2, ofm2, m12);
                    i8_st_tnsr_i_v(ofmIndex3, ofm3, m13);
                    ofmIndex0[2] += 1;
                    ofmIndex1[2] += 1;
                    ofmIndex2[2] += 1;
                    ofmIndex3[2] += 1;
                    i8_st_tnsr_i_v(ofmIndex0, ofm0, m20);
                    i8_st_tnsr_i_v(ofmIndex1, ofm1, m21);
                    i8_st_tnsr_i_v(ofmIndex2, ofm2, m22);
                    i8_st_tnsr_i_v(ofmIndex3, ofm3, m23);
                    ofmIndex0[2] += 1;
                    ofmIndex1[2] += 1;
                    ofmIndex2[2] += 1;
                    ofmIndex3[2] += 1;
                    i8_st_tnsr_i_v(ofmIndex0, ofm0, m30);
                    i8_st_tnsr_i_v(ofmIndex1, ofm1, m31);
                    i8_st_tnsr_i_v(ofmIndex2, ofm2, m32);
                    i8_st_tnsr_i_v(ofmIndex3, ofm3, m33);
                    ofmIndex0[2] -= 3;
                    ofmIndex1[2] -= 3;
                    ofmIndex2[2] -= 3;
                    ofmIndex3[2] -= 3;
                }
            }
        }
    }
}

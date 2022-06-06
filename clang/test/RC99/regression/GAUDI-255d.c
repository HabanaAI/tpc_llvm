// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 -o - %s
// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O2 -o - %s

// This is file failure_struct_in_st_tnsr.c fron GAUDI-255 attachments.

/*****************************************************************************
* Copyright (C) 2017 HabanaLabs, Ltd.
* All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Proprietary and confidential.
*
* Kernel assumptions:
* ====================
* input is a row major matrix
* output is a row major matrix
* index space is 1D, dictating which part of the input matrix is provided
* 
* Authors:
* Ron Shalev <rshalev@gaudilabs.com>
******************************************************************************
*/

#define K  4

__local__ char256   max_k_value[4];
__local__ ushort128 max_k_index1[4];
__local__ ushort128 max_k_index2[4];

typedef unsigned short uint16_t;


//compiler bug - can't pass pointers to function. should be fixed
typedef struct _char256_ushort256_pair_t
{
    char256 v1;
    ushort128 v21;
    ushort128 v22;
} char256_ushort256_pair_t;



void sort4Step(char256 unsorted[K], uchar256_char256_pair_t sorted[K], int k)
{
    //get maximum value
    sorted[k].v2 = unsorted[0];
    sorted[k].v1 = 0;
    sorted[k] = v_u8_sel2_grt_i8_b(unsorted[1], sorted[k].v2, 1, sorted[k].v1, 0, (uchar256_char256_pair_t){0}, 1, 0);
    sorted[k] = v_u8_sel2_grt_i8_b(unsorted[2], sorted[k].v2, 2, sorted[k].v1, 0, (uchar256_char256_pair_t){0}, 1, 0);
    sorted[k] = v_u8_sel2_grt_i8_b(unsorted[3], sorted[k].v2, 3, sorted[k].v1, 0, (uchar256_char256_pair_t){0}, 1, 0);

    //minimize maximum value
    unsorted[0] = v_i8_sel_eq_u8_b(sorted[k].v1, 0, -128, unsorted[0], 0, 0, 1, 0);
    unsorted[1] = v_i8_sel_eq_u8_b(sorted[k].v1, 1, -128, unsorted[1], 0, 0, 1, 0);
    unsorted[2] = v_i8_sel_eq_u8_b(sorted[k].v1, 2, -128, unsorted[2], 0, 0, 1, 0);
    unsorted[3] = v_i8_sel_eq_u8_b(sorted[k].v1, 3, -128, unsorted[3], 0, 0, 1, 0);
}


void sort4(char256 unsorted[K], uchar256_char256_pair_t sorted[K])
{
    sort4Step(unsorted, sorted, 0);
    sort4Step(unsorted, sorted, 1);
    sort4Step(unsorted, sorted, 2);
    sort4Step(unsorted, sorted, 3);
}




void main(tensor ifm, tensor ofm_val, tensor ofm_index, char firstActivation, char lastActivation)  
{
    int5 index_space_start = get_index_space_offset();
    int5 ofmValCord  = index_space_start;
    int5 ofmIndxCord = index_space_start;
    int5 index_space_end = get_index_space_size() + index_space_start;

    if (firstActivation==1)
    {     
        uchar256_char256_pair_t sorted_uchar[K];
        char256_ushort256_pair_t sorted[K];
        char256 unsorted[K];

        int5 ifmIndex = index_space_start;
        uint16_t baseIndx = index_space_start[0];

        unsorted[0] = v_i8_ld_tnsr_b(ifmIndex, ifm, 0, 0, 1, 0);
        ifmIndex[0]++;

        unsorted[1] = v_i8_ld_tnsr_b(ifmIndex, ifm, 0, 0, 1, 0);
        ifmIndex[0]++;

        unsorted[2] = v_i8_ld_tnsr_b(ifmIndex, ifm, 0, 0, 1, 0);
        ifmIndex[0]++;

        unsorted[3] = v_i8_ld_tnsr_b(ifmIndex, ifm, 0, 0, 1, 0);
        ifmIndex[0]++;

        sort4(unsorted, sorted_uchar);


        for (int i=0; i<K; i++) {
            sorted[i].v1 = sorted_uchar[i].v2;
            sorted[i].v21 = v_u16_and_b((ushort128)sorted_uchar[i].v1, 0xFFFF, 0, 0, 1, 0);
            sorted[i].v22 = v_u16_shr_b((ushort128)sorted_uchar[i].v1, 16, 0, 0, 1, 0);
            sorted[i].v22 = v_u16_and_b(sorted[i].v22, 0xFFFF, 0, 0, 1, 0);

        }
    
        ushort128 tmp = sorted[0].v21;
        v_i8_st_tnsr(ofmValCord, ofm_val, sorted_uchar[0].v2, 0, 1, 0);
        v_u16_st_tnsr(ofmIndxCord, ofm_index, tmp, 0, 1, 0);

        v_i8_st_tnsr(ofmValCord, ofm_val, sorted_uchar[1].v2, 0, 1, 0);
        v_u8_st_tnsr(ofmIndxCord, ofm_index, sorted_uchar[1].v1, 0, 1, 0);

        v_i8_st_tnsr(ofmValCord, ofm_val, sorted_uchar[2].v2, 0, 1, 0);
        v_u8_st_tnsr(ofmIndxCord, ofm_index, sorted_uchar[2].v1, 0, 1, 0);

        v_i8_st_tnsr(ofmValCord, ofm_val, sorted_uchar[3].v2, 0, 1, 0);
        v_u8_st_tnsr(ofmIndxCord, ofm_index, sorted_uchar[3].v1, 0, 1, 0);

    }
}

// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 -o - %s
// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O2 -o - %s

// This is max_4_val_and_idx_i8.c from GAUDI-255 attachments.

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
__local__ ushort128 max_k_index[4];

void main(tensor ifm, tensor ofm, char firstActivation, char lastActivation)  
{
    int5 index_space_start = get_index_space_offset();
    int5 index_space_end = get_index_space_size() + index_space_start;

    if (firstActivation==1)
    {
        for (int d = index_space_start[0] ; d <  index_space_end[0]; d += 1)
        {
            char256 unsorted[K];
            int5 ifmIndex = {d};
            for (int k=0; k<K; k++)
            {
            //    unsorted[k] = v_i8_ld_tnsr_b(ifmIndex, ifm, 0, 0, 1, 0);
            //    ifmIndex[0]++;
                unsorted[k] = 1;
            }

            char256 sum = 0;

            for (int k=0; k<K; k++)
            {
                sum += unsorted[k];
            }
            
            v_i8_st_tnsr(index_space_start, ofm, sum, 0, 1, 0);
        }
    }
}

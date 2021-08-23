// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o -

/*****************************************************************************
* Copyright (C) 2017 HabanaLabs, Ltd.
* All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Proprietary and confidential.
*
* Authors:
* Tzachi Cohen <tcohen@gaudilabs.com>
******************************************************************************
*/
void main(tensor a)
{
    int64 vlm_data[8];
    int64 a = 0 ;
    vlm_data[0] = a;
    a += 1;
    vlm_data[1] = a;
    a += 1;
    vlm_data[2] = a;
    a += 1;
    vlm_data[3] = a;
    a += 1;
    vlm_data[4] = a;
    a += 1;
    vlm_data[5] = a;
    a += 1;
    vlm_data[6] = a;
    a += 1;
    vlm_data[7] = a;

    int5 storeCoord = { 0, 1, 2, 3, 4 };
    for (int i = 0; i < 8; ++i)
        i32_st_tnsr_i_v_b(storeCoord, 1, vlm_data[i], 1, 0);
}

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
    int64 vlm_data[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    int5 storeCoord = { 0, 1, 2, 3, 4 };
    for (int i = 0; i < 8; ++i)
        v_i32_st_tnsr(storeCoord, 1, vlm_data[i], 0, 1, 0);
}

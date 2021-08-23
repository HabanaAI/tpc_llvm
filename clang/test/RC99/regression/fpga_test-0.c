// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

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
__local__ int64 vlm_data[5];
void main(tensor a)
{
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
}
// CHECK: st_l_v
// CHECK: st_l_v
// CHECK: st_l_v
// CHECK: st_l_v
// CHECK: st_l_v
// CHECK-NO: st_l_v

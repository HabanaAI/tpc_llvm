// RUN: %tpc_clang -S -O2 -g -o - %s
// RUN: %tpc_clang -S -O2 -g -DTAKEN -o - %s

/*****************************************************************************
* Copyright (C) 2017 HabanaLabs, Ltd.
* All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Proprietary and confidential.
*
* Authors:
* Sergei Gorenko <sergei.gorenko@p-product.com>
******************************************************************************
*/

// GAUDI-271

//#define INT8
//    #include "batch_norm.h"
//#undef INT8

void main(tensor ifm,
    tensor beta,
    tensor gammaScale,
    tensor gammaShift,
    tensor ofm)
{
    const int depth = 0;
    const int width = 1;
    const int height = 2;
    const int batch = 3;

    const int5 index_space_start = get_index_space_offset();
    const int5 index_space_end = get_index_space_size() + index_space_start;

    // DEPTH
    const int depthStep = 256;
    const int depthStart = index_space_start[depth] * depthStep;
    const int depthEnd = index_space_end[depth] * depthStep;

    // WIDTH
    const int widthStep = 4;
    const int widthStart = index_space_start[width] * widthStep;
    const int widthEnd = index_space_end[width] * widthStep;

    // HEIGHT
    const int heightStep = 1;
    const int heightStart = index_space_start[height];
    const int heightEnd = index_space_end[height];

    // BATCH
    const int batchStep = 1;
    const int batchStart = index_space_start[batch];
    const int batchtEnd = index_space_end[batch];

    const char zero = 0;
    const char mone = 1;

    int256 acc0, acc1, acc2, acc3, shift;
    char256 betaVal, gammaScaleVal, gammaShiftVal, y0, y1, y2, y3;
    char256 x0, x1, x2, x3;

    int5 ifmCoords = { 0, 0, 0, 0, 0 };
    int5 ofmCoords = { 0, 0, 0, 0, 0 };
    int5 depthCoords = { depthStart, 0, 0, 0, 0 };

    // Set Acc0, Acc1, Acc2, Acc3 values to 0
    acc0.v1 = 0;    acc0.v2 = 0;    acc0.v3 = 0;    acc0.v4 = 0;
    acc1.v1 = 0;    acc1.v2 = 0;    acc1.v3 = 0;    acc1.v4 = 0;
    acc2.v1 = 0;    acc2.v2 = 0;    acc2.v3 = 0;    acc2.v4 = 0;
    acc3.v1 = 0;    acc3.v2 = 0;    acc3.v3 = 0;    acc3.v4 = 0;

    for (int d = depthStart; d < depthEnd; d += depthStep)
    {
        ifmCoords[depth] = d;
        ofmCoords[depth] = d;

        // Clear gammaShiftVal.i32
        shift.v1 = 0;
        shift.v2 = 0;
        shift.v3 = 0;
        shift.v4 = 0;

        // Loading gammaScale, gammaShift and beta
        betaVal       = v_i8_ld_tnsr_b(depthCoords, beta, 0, 0, 1, 0);
        gammaScaleVal = v_i8_ld_tnsr_b(depthCoords, gammaScale, 0, 0, 1, 0);
        gammaShiftVal = v_i8_ld_tnsr_b(depthCoords, gammaShift, 0, 0, 1, 0);
        depthCoords[depth] += depthStep;

        // Convert gammaShiftVal.i8 to gammaShiftVal.i32
        //shift = v_i8_mac_b(gammaShiftVal, 1, shift, 1, 1, 0);

#ifdef TAKEN
	#pragma clang loop taken(enable)
#endif
        for (int b = batchStart; b < batchtEnd; b += batchStep)
        {
            ifmCoords[batch] = b;
            ofmCoords[batch] = b;

            for (int h = heightStart; h < heightEnd; h += heightStep)
            {
                ifmCoords[height] = h;
                ofmCoords[height] = h;

                ifmCoords[width] = widthStart;
                ofmCoords[width] = widthStart;

                // Loading input values
                x0 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;
                x1 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;
                x2 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;
                x3 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;

                // Multiplying column 0 by gammaScaleVal
                acc0 = v_i8_mac_b(x0, gammaScaleVal, acc0, 1, 1, 0);

                for (int w = widthStart; w < widthEnd; w += widthStep)
                {
                    // Multiplying column 1, 2, 3 by gammaScaleVal
                    acc1 = v_i8_mac_b(x1, gammaScaleVal, acc1, 1, 1, 0);
                    acc2 = v_i8_mac_b(x2, gammaScaleVal, acc2, 1, 1, 0);
                    acc3 = v_i8_mac_b(x3, gammaScaleVal, acc3, 1, 1, 0);

                    // acc0 << gammaShiftVal
                    acc0.v1 = v_i32_ash_b(acc0.v1, gammaShiftVal, 1 << 1, acc0.v1, 1, 0);
                    acc0.v2 = v_i32_ash_b(acc0.v2, gammaShiftVal, 1 << 1, acc0.v2, 1, 0);
                    acc0.v3 = v_i32_ash_b(acc0.v3, gammaShiftVal, 1 << 1, acc0.v3, 1, 0);
                    acc0.v4 = v_i32_ash_b(acc0.v4, gammaShiftVal, 1 << 1, acc0.v4, 1, 0);

                    // acc1 << gammaShiftVal
                    acc1.v1 = v_i32_ash_b(acc1.v1, gammaShiftVal, 1 << 1, acc1.v1, 1, 0);
                    acc1.v2 = v_i32_ash_b(acc1.v2, gammaShiftVal, 1 << 1, acc1.v2, 1, 0);
                    acc1.v3 = v_i32_ash_b(acc1.v3, gammaShiftVal, 1 << 1, acc1.v3, 1, 0);
                    acc1.v4 = v_i32_ash_b(acc1.v4, gammaShiftVal, 1 << 1, acc1.v4, 1, 0);

                    // acc2 << gammaShiftVal
                    acc2.v1 = v_i32_ash_b(acc2.v1, gammaShiftVal, 1 << 1, acc2.v1, 1, 0);
                    acc2.v2 = v_i32_ash_b(acc2.v2, gammaShiftVal, 1 << 1, acc2.v2, 1, 0);
                    acc2.v3 = v_i32_ash_b(acc2.v3, gammaShiftVal, 1 << 1, acc2.v3, 1, 0);
                    acc2.v4 = v_i32_ash_b(acc2.v4, gammaShiftVal, 1 << 1, acc2.v4, 1, 0);

                    // acc3 << gammaShiftVal
                    acc3.v1 = v_i32_ash_b(acc3.v1, gammaShiftVal, 1 /*RHU*/ << 1, 0, 1, 0);
                    acc3.v2 = v_i32_ash_b(acc3.v2, gammaShiftVal, 1 << 1, 0, 1, 0);
                    acc3.v3 = v_i32_ash_b(acc3.v3, gammaShiftVal, 1 << 1, 0, 1, 0);
                    acc3.v4 = v_i32_ash_b(acc3.v4, gammaShiftVal, 1 << 1, 0, 1, 0);

                    // acc + beta
                    acc0 = v_i8_mac_b(betaVal, mone, acc0, SW_SAT, 1, 0);
                    acc1 = v_i8_mac_b(betaVal, mone, acc1, SW_SAT, 1, 0);
                    acc2 = v_i8_mac_b(betaVal, mone, acc2, SW_SAT, 1, 0);
                    acc3 = v_i8_mac_b(betaVal, mone, acc3, SW_SAT, 1, 0);

                    // Load new input values
                    x0 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;
                    x1 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;
                    x2 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;
                    x3 = v_i8_ld_tnsr_b(ifmCoords, ifm, 0, 0, 1, 0); ifmCoords[width] += 1;

                    // int32_to_int8(acc0)
                    y0 = v_convert_int32_to_i8_b(acc0.v1, 0, /*lane*/ 0, SW_RHNE, y0, 1, 0);    acc0.v1 = zero;
                    y0 = v_convert_int32_to_i8_b(acc0.v2, 0, /*lane*/ 1, SW_RHNE, y0, 1, 0);    acc0.v2 = zero;
                    y0 = v_convert_int32_to_i8_b(acc0.v3, 0, /*lane*/ 2, SW_RHNE, y0, 1, 0);    acc0.v3 = zero;
                    y0 = v_convert_int32_to_i8_b(acc0.v4, 0, /*lane*/ 3, SW_RHNE, y0, 1, 0);    acc0.v4 = zero;
                    v_i8_st_tnsr(ofmCoords, ofm, y0, 0, 1, 0); ofmCoords[width] += 1;

                    // int32_to_int8(acc1)
                    y1 = v_convert_int32_to_i8_b(acc1.v1, 0, /*lane*/ 0, SW_RHNE, y1, 1, 0);    acc1.v1 = zero;
                    y1 = v_convert_int32_to_i8_b(acc1.v2, 0, /*lane*/ 1, SW_RHNE, y1, 1, 0);    acc1.v2 = zero;
                    y1 = v_convert_int32_to_i8_b(acc1.v3, 0, /*lane*/ 2, SW_RHNE, y1, 1, 0);    acc1.v3 = zero;
                    y1 = v_convert_int32_to_i8_b(acc1.v4, 0, /*lane*/ 3, SW_RHNE, y1, 1, 0);    acc1.v4 = zero;
                    v_i8_st_tnsr(ofmCoords, ofm, y1, 0, 1, 0); ofmCoords[width] += 1;

                    // int32_to_int8(acc2)
                    y2 = v_convert_int32_to_i8_b(acc2.v1, 0, /*lane*/ 0, SW_RHNE, y2, 1, 0);    acc2.v1 = zero;
                    y2 = v_convert_int32_to_i8_b(acc2.v2, 0, /*lane*/ 1, SW_RHNE, y2, 1, 0);    acc2.v2 = zero;
                    y2 = v_convert_int32_to_i8_b(acc2.v3, 0, /*lane*/ 2, SW_RHNE, y2, 1, 0);    acc2.v3 = zero;
                    y2 = v_convert_int32_to_i8_b(acc2.v4, 0, /*lane*/ 3, SW_RHNE, y2, 1, 0);    acc2.v4 = zero;
                    v_i8_st_tnsr(ofmCoords, ofm, y2, 0, 1, 0); ofmCoords[width] += 1;

                    // int32_to_int8(acc3)
                    y3 = v_convert_int32_to_i8_b(acc3.v1, 0, /*lane*/ 0, SW_RHNE, y3, 1, 0);    acc3.v1 = zero;
                    y3 = v_convert_int32_to_i8_b(acc3.v2, 0, /*lane*/ 1, SW_RHNE, y3, 1, 0);    acc3.v2 = zero;
                    y3 = v_convert_int32_to_i8_b(acc3.v3, 0, /*lane*/ 2, SW_RHNE, y3, 1, 0);    acc3.v3 = zero;
                    y3 = v_convert_int32_to_i8_b(acc3.v4, 0, /*lane*/ 3, SW_RHNE, y3, 1, 0);    acc3.v4 = zero;
                    v_i8_st_tnsr(ofmCoords, ofm, y3, 0, 1, 0); ofmCoords[width] += 1;

                    // Multiplying column 0 by gammaScaleVal
                    acc0 = v_i8_mac_b(x0, gammaScaleVal, acc0, 1, 1, 0);
                }
            }
        }
    }
}


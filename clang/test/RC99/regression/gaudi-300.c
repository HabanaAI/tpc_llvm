// RUN: %tpc_clang -S -O1 -o - %s
// RUN: %tpc_clang -S -O2 -o - %s

// This is beam_search.c

/*****************************************************************************
* Copyright (C) 2017 HabanaLabs, Ltd.
* All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Proprietary and confidential.
*
* Authors:
* Keren Luzon <kluzon@gaudilabs.com>
******************************************************************************
*/

//volatile __local__ short128_ushort128_pair_t localArray[2][2] = {{{-128, 0}, {-128, 0}}, {{-128, 0}, {-128, 0}}};

void main(tensor vocabulary,    // input_tensor  (int8)
        tensor output,
        tensor hypoScore,       // output_tensor (int8)
        tensor hypoIndices     // output_tensor (int16)
//        int bsw
    )
{    
    int5 index_space_start = get_index_space_offset();
    int5 index_space_end = get_index_space_size() + index_space_start;
    
    int5 loadCoords  = {0};
    int5 storeCoords = {0};
    
    ushort128_short128_pair_t localArray[2][2] = {{{0, -128}, {0, -128}}, {{0, -128}, {0, -128}}};
    
//    short128 maxScore[2][2] = {{-128, -128}, {-128, -128}};
//    ushort128 maxIndices[2][2] = {{0, 0}, {0, 0}};
    
    ushort128_short128_pair_t currScore0 = {0, -128};
    ushort128_short128_pair_t currScore1 = {0, -128};

    ushort128_short128_pair_t currScore2 = {0, -128};
    ushort128_short128_pair_t currScore3 = {0, -128};
  
    // iterate over vocabulary rows
    for (int row = index_space_start[0]; row < index_space_end[0]; row += 1)
    {
        loadCoords[1] = row;
        storeCoords[1] = row;
        
        char256 a = v_i8_ld_tnsr_b(loadCoords, vocabulary, 0, 0, 1, 0); 
        
        char256 group0 = v_i8_unpack_b(a, SW_GROUP_0 | SW_STRIDE_2 | SW_GROUP_HALF_0, group0, 1, 0);
        char256 group1 = v_i8_unpack_b(a, SW_GROUP_0 | SW_STRIDE_2 | SW_GROUP_HALF_0, group1, 1, 0);
        
        short128 group0_i16 = v_convert_i8_to_i16_b(group0, 0, 0, 1, 0);
        short128 group1_i16 = v_convert_i8_to_i16_b(group1, 0, 0, 1, 0);        
        
        short128 vec0 = group0_i16;
        
        group0_i16 = v_i16_mov_dual_group_b(group0_i16, 0xFFFFFFFF, 1, 2, MkWr(1, 1), group0_i16, 1, 0);
        group0_i16 = v_i16_mov_dual_group_b(group1_i16, 0xFFFFFFFF, 0, 1, MkWr(1, 1), group0_i16, 1, 0);     
        group0_i16 = v_i16_mov_dual_group_b(group1_i16, 0xFFFFFFFF, 1, 3, MkWr(1, 1), group0_i16, 1, 0);
        
        group1_i16 = v_i16_mov_dual_group_b(group1_i16, 0xFFFFFFFF, 2, 1, MkWr(1, 1), group1_i16, 1, 0);
        group1_i16 = v_i16_mov_dual_group_b(vec0, 0xFFFFFFFF, 2, 0, MkWr(1, 1), group1_i16, 1, 0);
        group1_i16 = v_i16_mov_dual_group_b(vec0, 0xFFFFFFFF, 3, 2, MkWr(1, 1), group1_i16, 1, 0);
       
        storeCoords[0] = 0;
        v_i16_st_tnsr(storeCoords, output, group0_i16, 0, 1, 0);

        storeCoords[0] = 128;
        v_i16_st_tnsr(storeCoords, output, group1_i16, 0, 1, 0);
        
        currScore0 = v_u16_sel2_grt_i16_b(group0_i16, currScore0.v2, row, currScore0.v1, 0, currScore0, 1, 0);
        currScore1 = v_u16_sel2_grt_i16_b(group1_i16, currScore0.v2, row, currScore1.v1, 0, currScore1, 1, 0);

        localArray[0][0].v1 = currScore0.v1;
        localArray[0][1].v1 = currScore1.v1;
        localArray[0][0].v2 = currScore0.v2;
        localArray[0][1].v2 = currScore1.v2;
        
//        localArray[0][0] = currScore0;
//        localArray[0][1] = currScore1;

//        for (int i = 1; i < 2; i++) 
//        {
//            
//            currScore0 = v_u16_sel2_grt_i16_b(localArray[i-1][0].v1, localArray[i][0].v1, localArray[i-1][0].v2, localArray[i][0].v2, 0, currScore0, 1, 0);
//            currScore1 = v_u16_sel2_grt_i16_b(localArray[i-1][1].v1, localArray[i][1].v1, localArray[i-1][1].v2, localArray[i][1].v2, 0, currScore1, 1, 0);
//
//            currScore2 = v_u16_sel2_leq_i16_b(localArray[i-1][0].v1, localArray[i][0].v1, localArray[i-1][0].v2, localArray[i][0].v2, 0, currScore2, 1, 0);
//            currScore3 = v_u16_sel2_leq_i16_b(localArray[i-1][1].v1, localArray[i][1].v1, localArray[i-1][1].v2, localArray[i][1].v2, 0, currScore3, 1, 0);
//
//            localArray[i][0] = currScore0;
//            localArray[i][1] = currScore1;
//            
//            localArray[i-1][0] = currScore2;
//            localArray[i-1][1] = currScore3;
//        }
    }
        
    for (int j = 0; j < 2; j++)
    {
        storeCoords[1] = j;
        
        storeCoords[0] = 0;
//        v_i16_st_tnsr(storeCoords, hypoScore, currScore0.v1, 0, 1, 0);
//        v_u16_st_tnsr(storeCoords, hypoIndices, currScore0.v2, 0, 1, 0);
        
        short128 temp = localArray[j][0].v2;
        ushort128 temp2 = localArray[j][0].v1;

        v_i16_st_tnsr(storeCoords, hypoScore, temp, 0, 1, 0);
        v_u16_st_tnsr(storeCoords, hypoIndices, temp2, 0, 1, 0);
        
        storeCoords[0] = 128;
//        v_i16_st_tnsr(storeCoords, hypoScore, currScore1.v1, 0, 1, 0);
//        v_u16_st_tnsr(storeCoords, hypoIndices, currScore1.v2, 0, 1, 0);
        
        temp = localArray[j][1].v2;
        temp2 = localArray[j][1].v1;
        
        v_i16_st_tnsr(storeCoords, hypoScore, temp, 0, 1, 0);
        v_u16_st_tnsr(storeCoords, hypoIndices, temp2, 0, 1, 0);
    }
    
    
}

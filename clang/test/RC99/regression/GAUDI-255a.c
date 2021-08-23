// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 -o - %s
// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O2 -o - %s

// This is insert_sort.c from GAUDI-255 attachments.

/*****************************************************************************
* Copyright (C) 2017 HabanaLabs, Ltd.
* All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Proprietary and confidential.
*
* Authors:
* Ron Shalev <rshalev@gaudilabs.com>
*
* Kernel assumptions:
* ====================
* input is a row major matrix
* output is a row major matrix
* index space is 1D, dictating which part of the input matrix is provided
* 
*
* Kernel State:
* ======================
* The state is saved between kerenl activations in VLM
* 	- K Max value INT8  - address VLM[0:K-1]
* 	- K Max index UINT8  - address VLM[K:2K-1]
* 	- K Max value INT16 - address VLM[2K:4K-1]
* 	- K Max index UINT16 - address VLM[4K:6K-1]
*   - The base 'h' of the current UINT8 indexes
*
* Algorithm:
* ======================
*  if (first activation)
*  {
*		sort the 1st k vectors
*		create the INT16 state
*  }
*
*  For every new vector
*		Load vector + sortInsert.INT8 
*
*  Every 256 vectors (or last activation) adjast k max value & index to INT16 and save in VLM
*
*  if (last activation)
*  {
*		store max value and index INT16 to memory
*  }
*
* Inputs
* ======================
* - 2D Matrix which is row major
* - 1st activation flag?
* - last activation flag?
* 
* Outputs
* ======================
* - INT16  2D Matrix which is row major. Holds max values. Hight K. Width = 256 elements
* - UINT16 2D Matrix which is row major. Hold max indexes. Hight K. Width = 256 elements
*
* Index space
* ======================
* - The index space is 1D and reflects the vertical location in the input tensor
* 
* 
******************************************************************************
*/

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#define K  4

__local__ short128_ushort128_pair_t l_sorted16[2][4];
__local__ uchar256_char256_pair_t   l_sorted8[4];
__local__ uint8_t l_indexU8;






void sort4Step(char256 unsorted[K], uchar256_char256_pair_t sorted[K], int k)
{
    //get maximum value
    sorted[k].v1 = 0;
    sorted[k].v2 = unsorted[0];

    //FIXME LOOP this
    sorted[k] = v_i8_u8_sel2_grt_v_v_v_v(unsorted[1], sorted[k].v2, 1, sorted[k].v1);
    sorted[k] = v_i8_u8_sel2_grt_v_v_v_v(unsorted[2], sorted[k].v2, 2, sorted[k].v1);
    sorted[k] = v_i8_u8_sel2_grt_v_v_v_v(unsorted[3], sorted[k].v2, 3, sorted[k].v1);

    //minimize maximum value
    //FIXME LOOP this
    unsorted[0] = v_u8_i8_sel_eq_v_s_v_v(sorted[k].v1, 0, -128, unsorted[0]);
    unsorted[1] = v_u8_i8_sel_eq_v_s_v_v(sorted[k].v1, 1, -128, unsorted[1]);
    unsorted[2] = v_u8_i8_sel_eq_v_s_v_v(sorted[k].v1, 2, -128, unsorted[2]);
    unsorted[3] = v_u8_i8_sel_eq_v_s_v_v(sorted[k].v1, 3, -128, unsorted[3]);
}


void sort4(char256 unsorted[K], uchar256_char256_pair_t sorted[K])
{
    sort4Step(unsorted, sorted, 0);
    sort4Step(unsorted, sorted, 1);
    sort4Step(unsorted, sorted, 2);
    sort4Step(unsorted, sorted, 3);
}



void ldStateFromVLM(uchar256_char256_pair_t sorted8[K])
{
    for (int i=0; i<K; i++)
    {
        sorted8[i].v1 = l_sorted8[i].v1;
        sorted8[i].v2 = l_sorted8[i].v2;
    }
}


void storeStateInVLM(uchar256_char256_pair_t sorted[K])
{
//    //for (int i=0; i<K; i++)
//    //{ 
//        i8_st_l_v_s_s_v(0,i*256, sorted[i].v1);
//        u16_st_l_v_s_s_v(0,1024 + 2*i*256, sorted[i].v21);
//        u16_st_l_v_s_s_v(0,1024 + (2*i+1)*256, sorted[i].v22);
//    //}
}

void ld4VectorsFromMemoryAndSort(uchar256_char256_pair_t sorted8[K], int5 ifmCord)
{
    char256 unsorted[K];
    char256 tmpChar256; 
    ushort128 tmpUshort128;

    unsorted[0] = v_i8_ld_tnsr_i(ifmCord,0);
    ifmCord[0]++;

    unsorted[1] = v_i8_ld_tnsr_i(ifmCord,0);
    ifmCord[0]++;

    unsorted[2] = v_i8_ld_tnsr_i(ifmCord,0);
    ifmCord[0]++;

    unsorted[3] = v_i8_ld_tnsr_i(ifmCord,0);
    ifmCord[0]++;

    sort4(unsorted, sorted8);

    for (int i=0; i<K; i++) {
    // Comment back in the following piece of code once compiler is fixed
    //	l_sorted16[0][i].v1 = v_convert_i8_to_i16_v(sorted8[i].v1, 0, 0); //why 3 arguments. FIXME compiler
    //	l_sorted16[0][i].v2 = v_u16_and_v_s((ushort128)sorted8[i].v2, 0xFFFF);
	//
    //	tmpChar256 = (char256)v_i16_shr_v_s((short128)sorted8[i].v1, 8); 
   	//	l_sorted16[1][i].v1 = v_convert_i8_to_i16_v(tmpChar256, 0, 0);
    //
    //  tmpUshort128 =  v_u16_shr_v_s((ushort128)sorted8[i].v2, 8);
	//  l_sorted16[1][i].v2 = v_u16_and_v_s(tmpUshort128, 0xFFFF);
    }
}


void insertSort(uchar256_char256_pair_t sorted8[K], char256 vecToAdd, uint8_t idx)
{
	bool256 biggerThenSorted8[K];

	//the vector which is bigger then minimal values can just change them.
	sorted8[0] = v_i8_u8_sel2_grt_v_v_v_v(vecToAdd, sorted8[0].v2, idx, sorted8[0].v1);

	//starting from 2nd index as the 1st one was covered
	for (int i=2; i<K; i++) 
	{
		biggerThenSorted8[i] =  bv_i8_cmp_grt_v_v(sorted8[i].v2, vecToAdd);
		char256 tmpForSwap = v_i8_mov_v_vb(sorted8[i].v2, tmpForSwap, biggerThenSorted8[i], 0);
		sorted8[i-1].v2    = v_i8_mov_v_vb(sorted8[i].v2, tmpForSwap, biggerThenSorted8[i], 0);
		sorted8[i].v2      = v_i8_mov_v_vb(sorted8[i-1].v2, tmpForSwap, biggerThenSorted8[i], 0);

		uchar256 utmpForSwap = v_u8_mov_v_vb(sorted8[i].v1, utmpForSwap, biggerThenSorted8[i], 0);
		sorted8[i-1].v1     = v_u8_mov_v_vb(sorted8[i].v1, utmpForSwap, biggerThenSorted8[i], 0);
		sorted8[i].v1       = v_u8_mov_v_vb(sorted8[i-1].v1, utmpForSwap, biggerThenSorted8[i], 0);
	}
}

void main(tensor ifm, tensor ofm_val, tensor ofm_index, char firstActivation, char lastActivation, char save16BitsState, uint16_t baseIndexuint16)  
{
    int5 index_space_start = get_index_space_offset();
    int5 ifmCord     = index_space_start;
    int5 ofmValCord  = index_space_start;
    int5 ofmIndxCord = index_space_start;
    int5 index_space_end = get_index_space_size() + index_space_start;
    uchar256_char256_pair_t sorted8[K];
    uint8_t indexU8;

    if (firstActivation==1)
    {     
        ld4VectorsFromMemoryAndSort(sorted8, ifmCord);
        indexU8 = 0;
    } else 
    {
        ldStateFromVLM(sorted8);
        indexU8  = l_indexU8;
    }  

    for (uint16_t h = ifmCord[0]; h<index_space_end[0]; h++)
    {
        char256 vecToAdd = v_i8_ld_tnsr_i(ifmCord,ifm);
        insertSort(sorted8, vecToAdd, indexU8);
    }

    storeStateInVLM(sorted8);
}

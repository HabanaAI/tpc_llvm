// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O2 %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck --check-prefix=CHECK-O1 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s

/*
void main(tensor ifm, tensor ofm, int i, int o)
{
	float64	accumulator = 0.0f;

	for (int i = 0; i < get_dim_size(ifm, 1); i++) {
		for (int j = 0; j < get_dim_size(ifm, 2); j++) {
			int5 ifm_offset = {1,1,i,j,1};
			//ifm_offset.x = i; ifm_offset.y=j; ifm_offset.zwq=0;
			__local__ float64 *pInput = (float64 *)i;// = a_gen_addr_i(ifm_offset, ifm);
			accumulator += *pInput;
		}
	}

	int5 ofm_offset = {0,0,0,0,0}; 
	ofm_offset.xyzwq=0;
	__local__ float64 *pOutput = (float64 *)o;// = a_gen_addr_i(ofm_offset, ofm);
	*pOutput = accumulator;
}
*/
#define WITH_RELU 1
void main(tensor in, tensor bias, tensor out)
{
	//  C    W    H    B    NA
	// [dim0,dim1,dim2,dim3,dim4];
	int5 curInputOutputIndex;
	int5 curBiasIndex = {0,0,0,0,0};
	// C iterator - in slices of 64

    int C_end = 0;
    C_end = s_i32_ld_l_s_b(0x8010 , C_end, 1, 1, 0);
    int W_end = 0;
    W_end = s_i32_ld_l_s_b(0x8018 , W_end, 1, 1, 0);
    int H_end = 0;
    H_end = s_i32_ld_l_s_b(0x8020 , H_end, 1, 1, 0);
    int B_end = 0;
    B_end = s_i32_ld_l_s_b(0x8028 , B_end, 1, 1, 0);


	/*
	int C_end = 64;
    int W_end = 4;
    int H_end = 1;
    int B_end = 1;
	*/
	for (int C_itr = 0; C_itr < C_end; C_itr+=64)
	{
		curInputOutputIndex[0] = C_itr;
		curBiasIndex[0] = C_itr;
		float64 biasValVec = 0;
		biasValVec = v_f32_ld_tnsr_i_b(curBiasIndex,bias, biasValVec, 1, 0);

		for(int B_itr = 0; B_itr < B_end; B_itr++)
		{				
			curInputOutputIndex[3] = B_itr;

			for (int H_itr = 0; H_itr < H_end; H_itr++)
			{
				curInputOutputIndex[2] = H_itr;
					
				for (int W_itr = 0; W_itr < W_end; W_itr++)
				{
					// W handing starts here
					curInputOutputIndex[1] = W_itr;
					float64 inputValVec = 0;
					inputValVec = v_f32_ld_tnsr_i_b(curInputOutputIndex,in, inputValVec, 1, 0);
					float64 resultValVec = inputValVec + biasValVec;
#ifdef WITH_RELU
			        resultValVec = v_f32_max_v_s_b(resultValVec, 0.0f, resultValVec, 1, 0);
#endif            
					f32_st_tnsr_i_v_b(curInputOutputIndex, out,  resultValVec, 1, 0);
				} // W loop
			} // H loop
		} // B loop
	} // C loop
}



/*
// Multiply first vector by 2 and store
void main(tensor in, tensor bias, tensor out)
{
  int5 offset = {0,0,0,0,0};
  float64 val = v_f32_ld_tnsr_i_b(in, offset, 1, 0);
  float64 res = val * 2.0f;
  f32_st_tnsr_i_v_b(out, offset, res, 1, 0);
  
}
*/


/*
void main(tensor ifm, tensor ofm, int i, int o)
{
	float64	accumulator = 0.0f;

	for (int i = 0; i < get_dim_size(ifm, 1); i++) {
		for (int j = 0; j < get_dim_size(ifm, 2); j++) {
			int5 ifm_offset = {1,1,i,j,1};
			//ifm_offset.x = i; ifm_offset.y=j; ifm_offset.zwq=0;
			__local__ float64 *pInput = (float64 *)i;// = a_gen_addr_i(ifm_offset, ifm);
			accumulator += *pInput;
		}
	}

	int5 ofm_offset = {0,0,0,0,0}; 
	ofm_offset.xyzwq=0;
	__local__ float64 *pOutput = (float64 *)o;// = a_gen_addr_i(ofm_offset, ofm);
	*pOutput = accumulator;
}
*/

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt

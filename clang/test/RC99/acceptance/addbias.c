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
			__local__ float64 *pInput = (float64 *)i;// = gen_addr(ifm_offset, ifm, 0, 0, 1, 0);
			accumulator += *pInput;
		}
	}

	int5 ofm_offset = {0,0,0,0,0}; 
	ofm_offset.xyzwq=0;
	__local__ float64 *pOutput = (float64 *)o;// = gen_addr(ofm_offset, ofm, 0, 0, 1, 0);
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
    C_end = s_i32_ld_l(0x8010 , 1, C_end, 1, 0);
    int W_end = 0;
    W_end = s_i32_ld_l(0x8018 , 1, W_end, 1, 0);
    int H_end = 0;
    H_end = s_i32_ld_l(0x8020 , 1, H_end, 1, 0);
    int B_end = 0;
    B_end = s_i32_ld_l(0x8028 , 1, B_end, 1, 0);


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
		biasValVec = v_f32_ld_tnsr_b(curBiasIndex, bias, 0, biasValVec, 1, 0);

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
					inputValVec = v_f32_ld_tnsr_b(curInputOutputIndex, in, 0, inputValVec, 1, 0);
					float64 resultValVec = inputValVec + biasValVec;
#ifdef WITH_RELU
			        resultValVec = v_f32_max_b(resultValVec, 0.0f, 0, resultValVec, 1, 0);
#endif            
					v_f32_st_tnsr(curInputOutputIndex, out, resultValVec, 0, 1, 0);
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
  float64 val = v_f32_ld_tnsr_b(in, offset, 0, 1, 0, );
  float64 res = val * 2.0f;
  v_f32_st_tnsr(out, offset, res, 0, 1, 0);
  
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
			__local__ float64 *pInput = (float64 *)i;// = gen_addr(ifm_offset, ifm, 0, 0, 1, 0);
			accumulator += *pInput;
		}
	}

	int5 ofm_offset = {0,0,0,0,0}; 
	ofm_offset.xyzwq=0;
	__local__ float64 *pOutput = (float64 *)o;// = gen_addr(ofm_offset, ofm, 0, 0, 1, 0);
	*pOutput = accumulator;
}
*/

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt

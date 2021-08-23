// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O2 %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck --check-prefix=CHECK-O1 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s
void main(tensor in0, tensor in1, tensor out)
{
	//  C    W    H    B    NA
	// [dim0,dim1,dim2,dim3,dim4];
	int5 curInputOutputIndex;
	// C iterator - in slices of 64
    int C_end = 64;
    int W_end = 3;
    int H_end = 3;
    int B_end = 1;

	for (int C_itr = 0; C_itr < C_end; C_itr+=64)
	{
		curInputOutputIndex[0] = C_itr;

		for(int B_itr = 0; B_itr < B_end; B_itr++)
		{				
			curInputOutputIndex[3] = B_itr;

			for (int H_itr = 0; H_itr < H_end; H_itr++)
			{
				curInputOutputIndex[2] = H_itr;
					
				for (int W_itr = 0; W_itr < W_end; W_itr++)
				{
					curInputOutputIndex[1] = W_itr;
					float64 input0ValVec = 0;
					input0ValVec = v_f32_ld_tnsr_i_b(curInputOutputIndex, in0, input0ValVec, 1, 0);
					float64 input1ValVec = 0;
					input0ValVec = v_f32_ld_tnsr_i_b(curInputOutputIndex, in1, input0ValVec, 1, 0);
					float64 resultValVec = input0ValVec + input1ValVec;
					f32_st_tnsr_i_v_b(curInputOutputIndex, out, resultValVec, 1, 0);
				} // W loop
			} // H loop
		} // B loop
	} // C loop
}

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt


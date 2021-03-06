// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O2 %s -o - | FileCheck %s
//*** GAUDI-227 *** R U N: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck --check-prefix=CHECK-O1 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s

void main(tensor in, tensor out_max, tensor out_maxidx)
{
	int windowH = 3;
	int windowW = 3;
	int strideH = 2;
	int strideW = 2;
	int paddingH = 0;
	int paddingW = 0;
	int dilationH = 1;
	int dilationW = 1;

	int C_end = 64;
	int W_end = 2;
	int H_end = 2;
	int B_end = 1;

	//	 C    W    H    B    NA
	//	[dim0,dim1,dim2,dim3,dim4];

	int5 curInputIndex;
	int5 curOutputIndex;

	float64 fltZero = 0.0f;
	float64 fltMax = 3.402823466e+38F;
	float64 fltMin = 0;
	fltMin = v_f32_sub_b(fltZero, fltMax, 0 << 1, fltMin, 0, 1);

	for (int C_itr = 0; C_itr < C_end; C_itr += 64)
	{
		curInputIndex[0] = C_itr;
		curOutputIndex[0] = C_itr;

		for (int B_itr = 0; B_itr < B_end; B_itr++)
		{
			curInputIndex[3] = B_itr;
			curOutputIndex[3] = B_itr;

			for (int H_itr = 0; H_itr < H_end; H_itr++)
			{
				curOutputIndex[2] = H_itr;

				int inputWindowStartH = H_itr * strideH - paddingH;

				for (int W_itr = 0; W_itr < W_end; W_itr++)
				{
					curOutputIndex[1] = W_itr;
					int inputWindowStartW = W_itr * strideW - paddingW;
					//float64 maxVal = fltMin;
					float64 maxVal = 0;
					maxVal = v_f32_mov_b(fltMin, 0, maxVal, 1, 0);
					int64 maxIdx = 0;

					// Window handling starts here
					for (int kH_itr = 0; kH_itr < windowH; kH_itr++)
					{
						curInputIndex[2] = inputWindowStartH + kH_itr;

						for (int kW_itr = 0; kW_itr < windowW; kW_itr++)
						{
							int idxInWindow = kH_itr * windowW + kW_itr;
							curInputIndex[1] = inputWindowStartW + kW_itr;
							float64 inputValVec = 0;
							inputValVec = v_f32_ld_tnsr_b(curInputIndex, in, 0, inputValVec, 1, 0);
							/*
							// This is the code I need but the intrinsics for sel2 aren't implemented yet. So I will come up with a different flavor of the code.
							float64_int64_pair_t respair = v_i32_sel2_grt_f32_b(inputValVec, maxVal, maxIdx, idxInWindow, 0, 1, 0, );
							maxVal = respair.a;
							maxIdx = respair.b;
							*/
							bool256 predGrt = 0;
							predGrt = from_bool64(v_f32_cmp_grt_b(inputValVec, maxVal, 0, to_bool64(predGrt), 1, 0));
							//float64 v_f32_mov_vb(float64 a, 0, bool256 predicate, to_bool64(bool predicatePolarity), );
							maxVal = v_f32_mov_vb(inputValVec, 0, maxVal, to_bool64(predGrt), 0);
							//int64 v_i32_mov_vb(int32_t a, 0, bool256 predicate, to_bool64(bool predicatePolarity), );
							maxIdx = v_i32_mov_vb(idxInWindow, 0, maxIdx, to_bool64(predGrt), 0);
						}
					}
					v_f32_st_tnsr(curOutputIndex, out_max, maxVal, 0, 1, 0);
					v_i32_st_tnsr(curOutputIndex, out_maxidx, maxIdx, 0, 1, 0);
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

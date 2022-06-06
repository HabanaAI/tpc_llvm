// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi  -mllvm -tpc-inc-merger=0 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2  -mllvm -tpc-inc-merger=0 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -mllvm -tpc-inc-merger=0 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -mllvm -tpc-inc-merger=0 %s -o - | FileCheck %s

void main(unsigned dest, tensor out) {
int5 indx = { 0, 0, 0, 0, 0 };
int __global *addr = gen_addr(indx, out, 0, 0, 1, 0);

// Scalar intrinsics.
{
	bf16 inc0 = 0;
 	bf16 __local *dptr = (bf16 __local *)dest;
	bf16 res0 = s_bf16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

indx[0]++;
addr = gen_addr(indx, out, 0, 0, 1, 0);

// Vector intrinsics.
{
	bfloat128 inc0 = 0;
 	bfloat128 __local *dptr = (bfloat128 __local *)dest;
	bfloat128 res0 = v_bf16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}
}
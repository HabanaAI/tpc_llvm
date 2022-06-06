// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(tensor out) {
int5 indx = { 0, 0, 0, 0, 0 };
int __global *addr = gen_addr(indx, out, 0, 0, 1, 0);

// Scalar intrinsics.
{
	bf16 val0 = 0;
	s_bf16_st_g(addr, val0, 0, 0, 1);
    // CHECK-DAG: st_g %AD{{[0-9]+}}, %S{{[0-9]+}}
}

}
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -mllvm -tpc-inc-merger=0 -target-cpu goya2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -mllvm -tpc-inc-merger=0 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -mllvm -tpc-inc-merger=0 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned dest, tensor out, int dimmask) {
int5 indx = { 0, 0, 0, 0, 0 };
int __global *addr = gen_addr(indx, out, 0, 0, 1, 0);

// Scalar intrinsics.
{
	half inc0 = 0;
 	half __local *dptr = (half __local *)dest;
	half res0 = s_f16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

// Vector intrinsics.
{
	half128 inc0 = 0;
 	half128 __local *dptr = (half128 __local *)dest;
	half128 res0 = v_f16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	int5 inc0 = 0;
 	int5 __local *dptr = (int5 __local *)dest;
	int5 res0 = i_i32_ld_g(addr, 0b00010, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g b00010 %I{{[0-9]+}}, %AD{{[0-9]+}}
}

{
	int5 inc0 = 0;
 	int5 __local *dptr = (int5 __local *)dest;
	int5 res0 = i_i32_ld_g(addr, dimmask, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %M{{[0-9]+}} %I{{[0-9]+}}, %AD{{[0-9]+}}

}


}
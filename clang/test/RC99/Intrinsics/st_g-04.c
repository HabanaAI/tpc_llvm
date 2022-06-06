// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(tensor out) {
int5 indx = { 0, 0, 0, 0, 0 };
int __global *addr = gen_addr(indx, out, 0, 0, 1, 0);

// Scalar intrinsics.
{
	minifloat val0 = 0;
	s_f8_st_g(addr, val0, SW_PD | SW_EV_HINT, 0, 1);
    // CHECK-DAG: st_g ev_hint pd %AD{{[0-9]+}}, %S{{[0-9]+}}

}

{
	minihalf val0 = 0;
	s_h8_st_g(addr, val0, SW_PD | SW_EV_HINT, 0, 1);
    // CHECK-DAG: st_g ev_hint pd %AD{{[0-9]+}}, %S{{[0-9]+}}

}

{
	half val0 = 0;
	s_f16_st_g(addr, val0, SW_PD | SW_EV_HINT, 0, 1);
    // CHECK-DAG: st_g ev_hint pd %AD{{[0-9]+}}, %S{{[0-9]+}}

}

}
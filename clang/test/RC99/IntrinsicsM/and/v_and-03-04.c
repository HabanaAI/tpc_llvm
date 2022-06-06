// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(int x0a, int x1a, int desta, int vpreda)
{
    float64 __local *x0_ptr = (float64 __local *)x0a;
    float64 __local *x1_ptr = (float64 __local *)x1a;
	float64 __local *dest_ptr = (float64  __local *)desta;
    bool64 __local *vpred_ptr = (bool64  __local *)vpreda;

	float64 x0 = *x0_ptr;
	float64 x1 = *x1_ptr;
	float64 res = *dest_ptr;
    bool64 vpred = *vpred_ptr;

    res = v_f32_and_vb(x0, x1, SW_ANDN, res, vpred, 0);
	*dest_ptr++ = res;
}
//CHECK-DAG: and.f32 andn %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

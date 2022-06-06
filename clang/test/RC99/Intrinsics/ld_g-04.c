// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned dest, tensor out, int vpredp) {
int5 indx = { 0, 0, 0, 0, 0 };
int __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
bool256 __local *vpred_ptr256 = (bool256 __local *)vpredp;
bool128 __local *vpred_ptr128 = (bool128 __local *)vpredp;
bool64 __local *vpred_ptr64 = (bool64 __local *)vpredp;
bool256 vpred256 = *vpred_ptr256++;
bool128 vpred128 = *vpred_ptr128++;
bool64 vpred64 = *vpred_ptr64++;

// Scalar intrinsics.
{
	minifloat inc0 = 0;
 	minifloat __local *dptr = (minifloat __local *)dest;
	minifloat res0 = s_f8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	minihalf inc0 = 0;
 	minihalf __local *dptr = (minihalf __local *)dest;
	minihalf res0 = s_h8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

// Vector intrinsics and scalar predicate.
{
	minifloat256 inc0 = 0;
 	minifloat256 __local *dptr = (minifloat256 __local *)dest;
	minifloat256 res0 = v_f8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	minihalf256 inc0 = 0;
 	minihalf256 __local *dptr = (minihalf256 __local *)dest;
	minihalf256 res0 = v_h8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}


// Vector intrinsics and vector predicate.
{
	minifloat256 inc0 = 0;
 	minifloat256 __local *dptr = (minifloat256 __local *)dest;
	minifloat256 res0 = v_f8_ld_g_vb(addr, 0, inc0, vpred256, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	minihalf256 inc0 = 0;
 	minihalf256 __local *dptr = (minihalf256 __local *)dest;
	minihalf256 res0 = v_h8_ld_g_vb(addr, 0, inc0, vpred256, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	float64 inc0 = 0;
 	float64 __local *dptr = (float64 __local *)dest;
	float64 res0 = v_f32_ld_g_vb(addr, 0, inc0, vpred64, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	int64 inc0 = 0;
 	int64 __local *dptr = (int64 __local *)dest;
	int64 res0 = v_i32_ld_g_vb(addr, 0, inc0, vpred64, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	uint64 inc0 = 0;
 	uint64 __local *dptr = (uint64 __local *)dest;
	uint64 res0 = v_u32_ld_g_vb(addr, 0, inc0, vpred64, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	short128 inc0 = 0;
 	short128 __local *dptr = (short128 __local *)dest;
	short128 res0 = v_i16_ld_g_vb(addr, 0, inc0, vpred128, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	ushort128 inc0 = 0;
 	ushort128 __local *dptr = (ushort128 __local *)dest;
	ushort128 res0 = v_u16_ld_g_vb(addr, 0, inc0, vpred128, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	char256 inc0 = 0;
 	char256 __local *dptr = (char256 __local *)dest;
	char256 res0 = v_i8_ld_g_vb(addr, 0, inc0, vpred256, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	uchar256 inc0 = 0;
 	uchar256 __local *dptr = (uchar256 __local *)dest;
	uchar256 res0 = v_u8_ld_g_vb(addr, 0, inc0, vpred256, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}

{
	bool256 inc0 = 0;
 	bool256 __local *dptr = (bool256 __local *)dest;
	bool256 res0 = v_i1_ld_g_vb(addr, 0, inc0, vpred256, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %VP{{[0-9]+}}, %AD{{[0-9]+}}, !%VP{{[0-9]+}}

}
}

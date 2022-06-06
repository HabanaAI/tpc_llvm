// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya   %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned dest, tensor out) {
int5 indx = { 0, 0, 0, 0, 0 };
int __global *addr = gen_addr(indx, out, 0, 0, 1, 0);

// Scalar intrinsics.
{
	float inc0 = 0;
 	float __local *dptr = (float __local *)dest;
	float res0 = s_f32_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	int inc0 = 0;
 	int __local *dptr = (int __local *)dest;
	int res0 = s_i32_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	unsigned int inc0 = 0;
 	unsigned int __local *dptr = (unsigned int __local *)dest;
	unsigned int res0 = s_u32_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	short inc0 = 0;
 	short __local *dptr = (short __local *)dest;
	short res0 = s_i16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	unsigned short inc0 = 0;
 	unsigned short __local *dptr = (unsigned short __local *)dest;
	unsigned short res0 = s_u16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	char inc0 = 0;
 	char __local *dptr = (char __local *)dest;
	char res0 = s_i8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	unsigned char inc0 = 0;
 	unsigned char __local *dptr = (unsigned char __local *)dest;
	unsigned char res0 = s_u8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %S{{[0-9]+}}, %AD{{[0-9]+}}

}

// Vector intrinsics
{
	float64 inc0 = 0;
 	float64 __local *dptr = (float64 __local *)dest;
	float64 res0 = v_f32_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	int64 inc0 = 0;
 	int64 __local *dptr = (int64 __local *)dest;
	int64 res0 = v_i32_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	uint64 inc0 = 0;
 	uint64 __local *dptr = (uint64 __local *)dest;
	uint64 res0 = v_u32_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	short128 inc0 = 0;
 	short128 __local *dptr = (short128 __local *)dest;
	short128 res0 = v_i16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	ushort128 inc0 = 0;
 	ushort128 __local *dptr = (ushort128 __local *)dest;
	ushort128 res0 = v_u16_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	char256 inc0 = 0;
 	char256 __local *dptr = (char256 __local *)dest;
	char256 res0 = v_i8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

{
	uchar256 inc0 = 0;
 	uchar256 __local *dptr = (uchar256 __local *)dest;
	uchar256 res0 = v_u8_ld_g(addr, 0, inc0, 0, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g %V{{[0-9]+}}, %AD{{[0-9]+}}

}

}
// RUN: tpc-clang -c  -S -O0 -march=goya2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(float in,
          int out)
{
   half __local *res0 = (half  __local *)out;

	half pVal;
	pVal = s_convert_f32_to_f16(in, SW_RHNE, 0, 1, 0);

	
*res0 =  pVal;
}
//CHECK-ASM: convert.f32 target_type=f16 rhne %S{{[0-9]+}}, %S{{[0-9]+}}

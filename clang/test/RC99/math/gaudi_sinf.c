// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -fsyntax-only -Wall -verify %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s
// expected-no-diagnostics
// XFAIL: *
// Gaudi-3
void main(int dest, float xx)
{
// The program calculates dest = sin(xx) for abs(xx) < 8000

    __local__ float *res_ptr = (float __local *) dest;
	
	const float FOPI = 1.27323954473516f;

	const float DP[3] = {0.78515625f, 2.4187564849853515625e-4f, 3.77489497744594108e-8f};
	const float sincoef[3] = {-1.9515295891E-4f, 8.3321608736E-3f, -1.6666654611E-1f};
	const float coscoef[3] = {2.443315711809948E-005f, -1.388731625493765E-003f, 4.166664568298827E-002f};

    float y, z, x = xx;
    int i, j;
    int sign = 1;
    x = s_f32_abs_s_b(xx, 1, x, 0);

    if( xx < 0 )
    {
        sign = -1;
    }

    j = FOPI * x; // integer part of x/(PI/4)
    y = j;

// map zeros to origin
    if( j & 1 )
    {
        j += 1;
        y += 1.0f;
    }

// octant modulo 360 degrees
    j &= 7;

// reflect in x axis
    if( j > 3)
    {
        sign = -sign;
        j -= 4;
    }

// Extended precision modular arithmetic
        x = ((x - y * DP[0]) - y * DP[1]) - y * DP[2];

    z = x * x;
    if( (j==1) || (j==2) )
    {
        y = coscoef[0];
        for (i = 1; i < 3; i++)
            y = y * z + coscoef[i];
        y *= z * z;
        y -= 0.5f * z;
        y += 1.0f;
    }
    else
    {
        y = sincoef[0];
		for (i = 1; i < 3; i++)
			y = y * z + sincoef[i];
        y *= z * x;
        y += x;
    }

    *res_ptr = sign < 0 ? -y : y;    // Is there the sign function??
}

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: HALT
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: HALT

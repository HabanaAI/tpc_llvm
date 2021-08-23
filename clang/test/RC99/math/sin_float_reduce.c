// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM-O1 %s
// expected-no-diagnostics
// GAUDI-3


// FOPI: 1.27323954473516f

// DP_const: 0.78515625f, 2.4187564849853515625e-4f, 3.77489497744594108e-8

// sincoef_const: -1.9515295891E-4f, 8.3321608736E-3f, -1.6666654611E-1f

// coscoef_const: 2.443315711809948E-005f, -1.388731625493765E-003f, 4.166664568298827E-002f

void main(int dest, float xx, int addr, int addr1, int addr2)
{
// The program calculates dest = sin(xx) for xx in [0; pi/4]

    __local__ volatile  float *res_ptr = (float __local *) dest;
	
    __local__ volatile float *DP = (float __local *) addr;
    __local__ volatile float *sincoef = (float __local *) addr1;
    __local__ volatile float *coscoef = (float __local *) addr2;
    
    DP[0] = 0.78515625f;
    DP[1] = 2.4187564849853515625e-4f;
    DP[3] = 3.77489497744594108e-8f;
    sincoef[0] = -1.9515295891E-4f;
    sincoef[1] = 8.3321608736E-3f;
    sincoef[2] = -1.6666654611E-1f;
    coscoef[0] = 2.443315711809948E-005f;
    coscoef[0] = -1.388731625493765E-003f;
    coscoef[0] = 4.166664568298827E-002f;

    float y, z, x = xx;
    int i, j;
    int sign = 1;

    float tmp = xx;
    x = s_f32_abs_s_b(tmp, x, 1, 0);

    if( xx < 0.0f )
    {
        sign = -1;
    }

//    j = FOPI * x; // integer part of x/(PI/4)
//    y = j;
    j = 0;
    y = 0.0f;

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

    *res_ptr = y;
}

// CHECK:main
// CHECK-ASM: main
// CHECK-ASM: halt
// CHECK-O1:main
// CHECK-ASM-O1: main
// CHECK-ASM-O1: halt


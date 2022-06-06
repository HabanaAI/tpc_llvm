// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0, int x1, int dest, int mask, _Bool pred) {
  int5 __local *dptr = (int5 __local *)dest;
  int5 res = 0;
  int5 a = { x0, x0,  0, x0, x0 };
  int5 b = { x1, x1, x1,  0,  0 };
  
  // %Ix - %Iy

  res = i_i32_sub(a, b, mask, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: sub.i32 %M{{[0-9]+}} %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}

  res = i_i32_sub(a, b, mask, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: sub.i32 %M{{[0-9]+}} %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}

  res = i_i32_sub(a, b, mask, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: sub.i32 %M{{[0-9]+}} %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}, !%SP{{[0-9]+}}
  
  // %Sx - %Iy

  res = i_i32_sub(x0, b, mask, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: sub.i32 %M{{[0-9]+}} %I{{[0-9]+}}, %S0, %I{{[0-9]+}}

  res = i_i32_sub(x1, b, mask, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: sub.i32  %M{{[0-9]+}} %I{{[0-9]+}}, %S1, %I{{[0-9]+}}, %SP{{[0-9]+}}

  // Imm - %Iy

  res = i_i32_sub(123, a, mask, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: sub.i32 %M{{[0-9]+}} %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}

  res = i_i32_sub(123, a, mask, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: sub.i32 %M{{[0-9]+}} %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}, %SP{{[0-9]+}}

  res = i_i32_sub(123, a, mask, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: sub.i32 %M{{[0-9]+}} %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}, !%SP{{[0-9]+}}
}

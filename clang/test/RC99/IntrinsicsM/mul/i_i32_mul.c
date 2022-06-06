// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0, int x1, int dest, _Bool pred) {
  int5 __local *dptr = (int5 __local *)dest;
  int5 res = 0;
  int5 a = { x0, x0,  0, x0, x0 };
  int5 b = { x1, x1, x1,  0,  0 };
  
  res = i_i32_mul(a, b, 0x1e, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i32 b11110 %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}

  res = i_i32_mul(a, b, 0x10, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32  b10000 %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}

  res = i_i32_mul(a, b, 0x08, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 b01000 %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}, !%SP{{[0-9]+}}

  res = i_i32_mul(a, 123, 0x01, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i32 b00001 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}

  res = i_i32_mul(a, 123, 0x02, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32 b00010 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}, %SP{{[0-9]+}}

  res = i_i32_mul(a, 123, 0x04, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 b00100 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}, !%SP{{[0-9]+}}
}

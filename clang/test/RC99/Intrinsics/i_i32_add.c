// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0, int x1, int dest, _Bool pred) {
  int5 __local *dptr = (int5 __local *)dest;
  int5 res = 0;
  int5 a = { x0, x0,  0, x0, x0 };
  int5 b = { x1, x1, x1,  0,  0 };
  
  res = i_i32_add(a, b, 0b11110, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: add.i32 b11110 %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}

  res = i_i32_add(a, b, 0b10000, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: add.i32  b10000 %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}

  res = i_i32_add(a, b, 0b1000, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: add.i32 b01000 %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}, !%SP{{[0-9]+}}

  res = i_i32_add(a, x0, 0b00001, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: add.i32 b00001 %I{{[0-9]+}}, %S0, %I{{[0-9]+}}

  res = i_i32_add(a, x0, 0b00010, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: add.i32 b00010 %I{{[0-9]+}}, %S0, %I{{[0-9]+}}, %SP{{[0-9]+}}

  res = i_i32_add(a, x0, 0b00100, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: add.i32 b00100 %I{{[0-9]+}}, %S0, %I{{[0-9]+}}, !%SP{{[0-9]+}}

  res = i_i32_add(a, 123, 0b00001, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: add.i32 b00001 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}

  res = i_i32_add(a, 123, 0b00010, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: add.i32 b00010 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}, %SP{{[0-9]+}}

  res = i_i32_add(a, 123, 0b00100, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: add.i32 b00100 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}, !%SP{{[0-9]+}}
}

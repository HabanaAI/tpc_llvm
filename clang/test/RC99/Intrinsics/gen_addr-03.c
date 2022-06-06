// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(tensor out1, int out2, int value, _Bool p) {
// CHECK:      .globl main
  int5 indx = { 0, 0, 0, 0, 0 };
  int __global *ptr1 = gen_addr(indx, out2, 0, 0, 1, 0);
  *ptr1 = value;

// CHECK-DAG:  set_indx [[NDX:%I[0-9]+]], b11111, 0x0
// CHECK-DAG:  mov %ST_TNSR_ID_REG, %S0
// CHECK:      gen_addr dt=int8 [[ADDR:%AD[0-9]+]], %ST_TNSR_ID_REG, [[NDX]]
// CHECK:      st_g     [[ADDR]], %S1

  indx[0] = 1;
  int __global *ptr2 = gen_addr(indx, out1, SW_INT32, ptr1, p, 0);
  *ptr2 = value;

// CHECK: gen_addr  dt=int32 [[ADDR]], 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
// CHECK: st_g      [[ADDR]], %S1

  indx[0] = 2;
  int __global *ptr3 = gen_addr(indx, out1, SW_INT32 | SW_DT_OVERRIDE, ptr2, p, 0);
  *ptr3 = value;

// CHECK: gen_addr  dt=int32 dt_override [[ADDR]], 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
// CHECK: st_g      [[ADDR]], %S1

// CHECK:      halt
}

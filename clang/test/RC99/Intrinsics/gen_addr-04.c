// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor out1, int out2, int value) {
  int5 indx = { 0, 0, 0, 0, 0 };
  int __global *ptr1 = gen_addr(indx, out2, SW_PAD, 0, 1, 0);
  *ptr1 = value;

// CHECK: gen_addr dt=int8 pad %AD{{[0-9]+}}, %ST_TNSR_ID_REG, %I{{[0-9]+}}
}

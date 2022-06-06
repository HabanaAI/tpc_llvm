// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor out, unsigned dest, unsigned addr) {
  int5 indx = { 0, 0, 0, 0, 0 };
  void __global *ptr = gen_addr(indx, out);
  // CHECK: gen_addr dt=int8 [[ADDR:%AD[0-9]+]], 0x0, %I{{[0-9]+}}

  uint32_t_pair_t value = {0x100};
  update_addr(&ptr, value);
  // CHECK: mov  [[ADDR]], %Z{{[0-9]+}}
  unsigned __global *typed_ptr = (unsigned __global *)ptr;
  *typed_ptr = 0x10;
}

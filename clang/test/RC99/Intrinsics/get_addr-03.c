// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor out, unsigned dest, unsigned addr) {
  int5 indx = { 0, 0, 0, 0, 0 };
  void __global *ptr = gen_addr(indx, out); 

  volatile unsigned __local *dest_ptr = (unsigned __local *)dest;
  uint32_t_pair_t value = get_addr(ptr);
  // CHECK: mov  %Z{{[0-9]+}}, %AD{{[0-9]+}}
  *dest_ptr++ = value.v1;
}

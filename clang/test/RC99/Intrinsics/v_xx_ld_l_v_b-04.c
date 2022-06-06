// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned addr, int dest, _Bool pred) {
  int64 __local *vptr = (int64 __local *)dest;
  
  {
    minifloat256 __local *dptr = (minifloat256 __local *)vptr;
    minifloat256 result = 0;

    result = v_f8_ld_l_v_b(addr, 0, result, pred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}
    *dptr++ = result;
    
    result = v_f8_ld_l_v_b(0x100, 0, result, pred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, 0x100, %SP{{[0-9]+}}
    *dptr++ = result;
  }

  {
    minihalf256 __local *dptr = (minihalf256 __local *)vptr;
    minihalf256 result = 0;

    result = v_h8_ld_l_v_b(addr, 0, result, pred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}
    *dptr++ = result;
    
    result = v_h8_ld_l_v_b(0x100, 0, result, pred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, 0x100, %SP{{[0-9]+}}
    *dptr++ = result;
  }
}

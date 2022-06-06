// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned addr, int dest, int vpredp) {
  int64 __local *vptr = (int64 __local *)dest;

  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  bool256 vpred = *vpred_ptr;
  
  {
    minifloat256 __local *dptr = (minifloat256 __local *)vptr;
    minifloat256 result = 0;

    result = v_f8_ld_l_v_vb(addr, 0, result, vpred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, %S0, %VP{{[0-9]+}}
    *dptr++ = result;
    
    result = v_f8_ld_l_v_vb(0x100, 0, result, vpred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, 0x100, %VP{{[0-9]+}}
    *dptr++ = result;
  }

  {
    minihalf256 __local *dptr = (minihalf256 __local *)vptr;
    minihalf256 result = 0;

    result = v_h8_ld_l_v_vb(addr, 0, result, vpred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, %S0, %VP{{[0-9]+}}
    *dptr++ = result;
    
    result = v_h8_ld_l_v_vb(0x100, 0, result, vpred, 0);
    // CHECK: ld_l_v  %V{{[0-9]+}}, 0x100, %VP{{[0-9]+}}
    *dptr++ = result;
  }
}

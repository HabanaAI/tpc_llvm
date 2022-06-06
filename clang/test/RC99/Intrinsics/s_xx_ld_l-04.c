// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck %s

void main(unsigned addr, int dest, _Bool pred) {
  volatile float __local *dptr = (int __local *)dest;

  {
    minifloat res = 0;

    res = s_f8_ld_l(addr, 0, res, pred, 0);
	// CHECK: %S{{[0-9]+}}, %S0, %SP{{[0-9]+}}
    *dptr++ = res;
    
    res = s_f8_ld_l(0x100, 0, res, pred, 0);
    // CHECK: %S{{[0-9]+}}, 0x100, %SP{{[0-9]+}}
    *dptr++ = res;
  }

  {
    minihalf res = 0;

    res = s_h8_ld_l(addr, 0, res, pred, 0);
	// CHECK: %S{{[0-9]+}}, %S0, %SP{{[0-9]+}}
    *dptr++ = res;
    
    res = s_h8_ld_l(0x100, 0, res, pred, 0);
    // CHECK: %S{{[0-9]+}}, 0x100, %SP{{[0-9]+}}
    *dptr++ = res;
  }
}

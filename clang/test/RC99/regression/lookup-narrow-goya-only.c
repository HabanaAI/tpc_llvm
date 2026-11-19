// RUN: %tpc_clang -S -O2 -march=goya %s -o - | FileCheck %s
// RUN: not %tpc_clang -S -O2 -march=gaudi %s -o - 2>&1 | FileCheck %s --check-prefix=ERR
// RUN: not %tpc_clang -S -O2 -march=gaudi2 %s -o - 2>&1 | FileCheck %s --check-prefix=ERR
// RUN: not %tpc_clang -S -O2 -march=gaudi3 %s -o - 2>&1 | FileCheck %s --check-prefix=ERR

// LOOKUP of 16- and 8-bit data (SW_BV16_*, SW_BV8_*) exists only on Goya;
// from Gaudi on the 16-bit lookups are LOOKUP_1C/2C, and Habana's compiler
// does not declare v_i16_lookup, v_u16_lookup or v_i8_lookup there. The fork
// used to accept them on every target.

void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  ushort128 a = v_u16_ld_tnsr_b(c, in);
  short128 r = v_i16_lookup(a, 0, 0, (short128)0);
  v_i16_st_tnsr(c, out, r);
}

// CHECK: lookup
// ERR:   error: 'v_i16_lookup' needs target feature goya

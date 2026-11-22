// A MOV to LD_PARTIAL_REG needs 2 cycles before a partial ld_g reads it, and
// a MOV to M0-M3 needs 3 before an ld_g with a mask register: on the
// simulator, closer loads used the old size or mask. The first rule covered
// only tensor accesses, and the second never matched (the MOV's register
// class is HSRF, not MRF).
//
// RUN: %tpc_clang -march=gaudi2 -O2 -no-instr-compress -mllvm -tpc-asm-format=3 -S %s -o %t2.s
// RUN: %python -c "import re, sys; L = open(sys.argv[1]).read().splitlines(); f = lambda p, s=0: next(i for i in range(s, len(L)) if re.search(p, L[i])); p = f(r'mov\s+LD_PARTIAL_REG'); m = f(r'mov\s+M0,'); sys.exit(not (f(r'ld_g.*LD_PARTIAL_REG', p) - p >= 2 and f(r'ld_g.*\bM0 I', m) - m >= 3))" %t2.s
// RUN: %tpc_clang -march=gaudi3 -O2 -no-instr-compress -mllvm -tpc-asm-format=3 -S %s -o %t3.s
// RUN: %python -c "import re, sys; L = open(sys.argv[1]).read().splitlines(); f = lambda p, s=0: next(i for i in range(s, len(L)) if re.search(p, L[i])); p = f(r'mov\s+LD_PARTIAL_REG'); m = f(r'mov\s+M0,'); sys.exit(not (f(r'ld_g.*LD_PARTIAL_REG', p) - p >= 2 and f(r'ld_g.*\bM0 I', m) - m >= 3))" %t3.s

void main(tensor in, tensor masks, tensor out) {
  int5 r = {0, 0, 0, 0, 0};
  void __global *a = gen_addr(r, in, 0, 0, 1, 0);
  int64 v = 0;
  v = v_i32_ld_g_partial_b(a, 15, 0, 0, v, 1, 0);

  int5 m = {0, 0, 0, 0, 0};
  int mask = s_i32_ld_g((int __global *)gen_addr(m, masks, 0, 0, 1, 0), 0, 0, 1, 0);
  int5 z = 0;
  z = i_i32_ld_g_inc(&a, mask, 0, z, 1, 0);

  int5 o = {0, 0, 0, 0, 0};
  v_i32_st_tnsr(o, out, v + z[0]);
}

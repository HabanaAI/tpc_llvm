// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSTG %s -o - | FileCheck %s --check-prefix=FLUSH
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DSTG %s -o - | FileCheck %s --check-prefix=FLUSH
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSTG_LOOP %s -o - | FileCheck %s --check-prefix=FLUSH
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSTG_BRANCH %s -o - | FileCheck %s --check-prefix=FLUSH
// RUN: %tpc_clang -S -O2 -march=gaudi3 -DSTG %s -o - | FileCheck %s --check-prefix=FLUSH
// RUN: %tpc_clang -S -O2 -march=gaudi3 -DLDG %s -o - | FileCheck %s --check-prefix=FLUSH
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DLDG %s -o - | FileCheck %s --check-prefix=NONE
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DUSER_FLUSH %s -o - | FileCheck %s --check-prefix=USER
// RUN: %tpc_clang -S -O2 -march=greco -DSTG %s -o - | FileCheck %s --check-prefix=NONE
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSTG -mllvm -tpc-cache-flush-before-halt=false %s -o - | FileCheck %s --check-prefix=NONE

// Habana's compiler ends a gaudi2 kernel that stores to global memory
// (st_g), and a gaudi3 kernel that loads or stores there, with one
// unpredicated cache_flush right before halt, unless the kernel already
// flushes somewhere. Greco gets none.

void main(tensor out, int n, int m) {
  int5 c = 0;
  __global__ int *p = (__global__ int *)gen_addr(c, out);
#if defined(STG)
  s_i32_st_g(p, n);
#elif defined(STG_LOOP)
  for (int i = 0; i < n; i++) {
    c[0] = i;
    s_i32_st_g((__global__ int *)gen_addr(c, out), i);
  }
#elif defined(STG_BRANCH)
  if (n > m)
    s_i32_st_g(p, n);
  else
    v_i32_st_tnsr(c, out, (int64)n);
#elif defined(LDG)
  v_i32_st_tnsr(c, out, (int64)s_i32_ld_g(p));
#elif defined(USER_FLUSH)
  s_i32_st_g(p, n);
  cache_flush();
#endif
}

// FLUSH-LABEL: main:
// FLUSH:       cache_flush
// FLUSH-NOT:   cache_flush
// FLUSH:       halt

// NONE-LABEL:  main:
// NONE-NOT:    cache_flush
// NONE:        halt

// USER-LABEL:  main:
// USER:        st_g
// USER:        cache_flush
// USER-NOT:    cache_flush
// USER:        halt

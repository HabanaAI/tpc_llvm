// RUN: %tpc_clang -S -O2 -march=gaudi -Xclang -float8 %s -o - | FileCheck %s
// RUN: %tpc_clang -S -O2 -march=gaudib -Xclang -float8 %s -o - | FileCheck %s
// RUN: %tpc_clang -S -O2 -march=greco -Xclang -float8 %s -o - | FileCheck %s
// RUN: not %tpc_clang -S -O2 -march=gaudi -Xclang -float8 -DUSE_F8 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR

// -float8 on a target without 8-bit float types: the headers do not declare
// them, so they are not available. This used to assert in
// ASTContext::loadTpcTypes; Habana's compiler accepts it.

void main(tensor out) {
  int5 c = {0, 0, 0, 0, 0};
#ifdef USE_F8
  minifloat m = 1;
#endif
  v_f32_st_tnsr(c, out, 1.0f);
}

// CHECK: st_tnsr
// ERR: error: use of undeclared identifier 'minifloat'

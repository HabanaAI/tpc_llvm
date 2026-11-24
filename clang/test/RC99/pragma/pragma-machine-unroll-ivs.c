// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DPIPELINED %s -o - 2>&1 | FileCheck --check-prefix=PIPE %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DACC %s -o - 2>&1 | FileCheck --check-prefix=ACC %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DFLOAT_IV %s -o - 2>&1 | FileCheck --check-prefix=FIV %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DFLOAT_IV_ROWS %s -o - 2>&1 | FileCheck --check-prefix=FIVR %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O2 -target-cpu gaudi2 -DPRED %s -o - 2>&1 | FileCheck --check-prefix=PRED %s

// The backend machine unroll (TPCSoftwareLoopPipelining), as Habana's compiler
// does it:
// - pipelined(enable) is not applied, with Habana's warning. Pipelining used
//   values defined in the loop on the path that skips it (LiveVariables
//   asserted on any tensor loop with a runtime trip count).
// - A loop whose carried values include one that is not a linear induction
//   variable (here an accumulator) is not the backend unroller's: each copy
//   got its own accumulator and the result was wrong. Habana's compiler
//   refuses it with a warning. TPCPragmaChainUnroll gives it to LLVM's
//   unroller instead, which unrolls it by the pragma's count in order, with a
//   remainder loop (a kernel loop stepping by 4, then one by 1).
// - An f32 induction variable is unrolled with its step multiplied in f32:
//   copies start at k + 0.1f, k + 0.2f, k + 0.3f and step by 0.4f. The bits of
//   0.1f were multiplied by 4 as an integer before.
// - A loop with the pragma is left to the backend unroller. The IR unroller
//   unrolled one that carries a value (an f32 induction variable is one there)
//   when it knew the trip count (-DFLOAT_IV_ROWS), each copy adding 0.1f to
//   the previous one, and the backend then warned that the pragma was not
//   applied.
// - A predicated add ('count += 1' in the lanes where a compare holds) is not
//   a linear induction variable either. It was taken for one, and each copy
//   counted every fourth iteration. It is unrolled in order as well.

void main(tensor y, tensor out, int hidden) {
  int5 c = 0;
#if defined(PIPELINED)
  #pragma clang loop machine_unroll_count(4) pipelined(enable)
  for (int x = 0; x < hidden; x += 128) {
    c[0] = x;
    v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, y));
  }
#elif defined(ACC)
  float64 acc = 0;
  #pragma clang loop machine_unroll_count(4)
  for (int x = 0; x < hidden; x += 128) {
    c[0] = x;
    acc = acc + v_f32_ld_tnsr_b(c, y);
  }
  c[0] = 0;
  v_f32_st_tnsr(c, out, acc);
#elif defined(FLOAT_IV)
  float64 k = 1.0f;
  #pragma clang loop machine_unroll_count(4)
  for (int x = 0; x < hidden; x += 128) {
    c[0] = x;
    v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, y) * k);
    k = k + 0.1f;
  }
#elif defined(FLOAT_IV_ROWS)
  float64 k = 1.0f;
  #pragma clang loop machine_unroll_count(4)
  for (int r = 0; r < hidden; r++) {
    c[1] = r;
    v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, y) * k);
    k = k + 0.1f;
  }
#elif defined(PRED)
  int64 count = 0;
  #pragma clang loop machine_unroll_count(4)
  for (int x = 0; x < hidden; x += 64) {
    c[0] = x;
    bool64 above = v_f32_cmp_grt_b(v_f32_ld_tnsr_b(c, y), 0.5f);
    count = v_i32_add_vb(count, 1, 0, count, above, 0);
  }
  c[0] = 0;
  v_i32_st_tnsr(c, out, count);
#endif
}

// PIPE:      warning: pipelining is not applied to the loop.
// PIPE-NEXT: Pipelining is not supported now
// PIPE:      loop 0, %S0, 512, <,
// PIPE-COUNT-4: st_tnsr
// PIPE:      halt

// ACC-NOT:  warning
// ACC:      loop %S{{[0-9]+}}, 0, -4, >,
// ACC:      loop %S{{[0-9]+}}, 0, -1, >,
// ACC:      add.f32
// ACC:      halt

// FIV-NOT:  warning
// FIV:      mov.f32 [[K:%V[0-9]+]], 0x3f800000
// FIV-DAG:  add.f32 %V{{[0-9]+}}, [[K]], 0x3dcccccd
// FIV-DAG:  add.f32 %V{{[0-9]+}}, [[K]], 0x3e4ccccd
// FIV-DAG:  add.f32 %V{{[0-9]+}}, [[K]], 0x3e99999a
// FIV:      loop 0, %S0, 512, <,
// FIV-COUNT-4: add.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3ecccccd
// FIV:      halt

// FIVR-NOT:  warning
// FIVR:      mov.f32 [[K:%V[0-9]+]], 0x3f800000
// FIVR-DAG:  add.f32 %V{{[0-9]+}}, [[K]], 0x3dcccccd
// FIVR-DAG:  add.f32 %V{{[0-9]+}}, [[K]], 0x3e4ccccd
// FIVR-DAG:  add.f32 %V{{[0-9]+}}, [[K]], 0x3e99999a
// FIVR:      loop 0, %S0, 4, <,
// FIVR-COUNT-4: add.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3ecccccd
// FIVR:      halt

// PRED-NOT:  warning
// PRED:      loop %S{{[0-9]+}}, 0, -4, >,
// PRED:      loop %S{{[0-9]+}}, 0, -1, >,
// PRED:      add.i32 {{.*}}, %VP{{[0-9]+}}
// PRED:      halt

// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-modulo-schedule=false -mllvm -tpc-chain-unroll-budget=250 %s -o - | FileCheck %s --check-prefix=BUDGET
// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-modulo-schedule=false -mllvm -tpc-chain-unroll-budget=0 %s -o - | FileCheck %s --check-prefix=ALL

// The IR unroller unrolls each loop that carries a value (here a MAC
// accumulator) by 4, with a remainder loop, as long as the function stays
// within -tpc-chain-unroll-budget IR instructions: a kernel whose code grows
// past the TPC's ~64 KB instruction cache runs several times slower on the
// card (GaudiInfer gdn_state_commit_f32: 16 such loops took it from 53 to
// 104 KB and 4.2x the card time). With a small budget the first loops are
// unrolled and the later ones are left as they are; with no budget, all are.

#define L(k)                                                                  \
  {                                                                           \
    float64 acc = 0;                                                          \
    for (int t = 0; t < n; t++) {                                             \
      c[1] = t;                                                               \
      c[2] = k;                                                               \
      acc = v_f32_mac_b(acc, acc, v_f32_ld_tnsr_b(c, in));                    \
    }                                                                         \
    c[2] = k;                                                                 \
    v_f32_st_tnsr(c, out, acc);                                               \
  }

void main(tensor in, tensor out, int n) {
  int5 c = 0;
  L(0) L(1) L(2) L(3) L(4) L(5) L(6) L(7)
}

// BUDGET:     loop 0, %S{{[0-9]+}}, 4, <,
// BUDGET:     loop %S{{[0-9]+}}, 0, -1, >,
// BUDGET:     loop 0, %S{{[0-9]+}}, 4, <,
// BUDGET:     loop %S{{[0-9]+}}, 0, -1, >,
// BUDGET-NOT: , 4, <,
// BUDGET:     loop 0, %S{{[0-9]+}}, 1, <,

// ALL-NOT:    loop 0, %S{{[0-9]+}}, 1, <,
// ALL-COUNT-7: loop %S{{[0-9]+}}, 0, -1, >,

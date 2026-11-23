// RUN: %tpc_clang -S -O2 -march=gaudi2 -DDOUBLE %s -o - | FileCheck %s --check-prefix=DOUBLE
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DLOWZERO %s -o - | FileCheck %s --check-prefix=LOWZERO
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSAMEHIGH %s -o - | FileCheck %s --check-prefix=SAMEHIGH
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DSAMEHIGH %s -o - | FileCheck %s --check-prefix=SAMEHIGH
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DDEADLOW %s -o - | FileCheck %s --check-prefix=DEADLOW
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DDEADLOW %s -o - | FileCheck %s --check-prefix=DEADLOW
// RUN: %tpc_clang -S -O2 -march=gaudi -DSCALAR %s -o - | FileCheck %s --check-prefix=SCALAR
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DLOOP %s -o - | FileCheck %s --check-prefix=LOOP
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSUBDEADLOW %s -o - | FileCheck %s --check-prefix=SUBDEADLOW
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DSUBDEADLOW %s -o - | FileCheck %s --check-prefix=SUBDEADLOW
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSUBZERO %s -o - | FileCheck %s --check-prefix=SUBZERO
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSUBSAMEHIGH %s -o - | FileCheck %s --check-prefix=SUBSAMEHIGH
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSUBSCALAR %s -o - | FileCheck %s --check-prefix=SUBSCALAR

// The carry an ADD with CARRY reads is not a value in the IR, so passes
// could remove, merge, fold or move the add before it, or merge two equal
// carry reads, leaving the high word of a 64-bit add with another add's
// carry (both compilers, checked on the simulator). TPCCarryPairs now keeps
// each carry read and the add before it as one operation.
//
// The SUB* cases are a 64-bit subtract: SUB writes the carry as (a >= b),
// unsigned, and SUB with BORROW subtracts that flag as it is (simulator and
// gaudi2 card agree), so hi = c - d - borrow is computed as an ADD with
// CARRY of ~d after the low SUB. The pair is kept the same way.

#if defined(DOUBLE)
// x + x was folded into a shift, which writes no carry.
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 b = v_i32_ld_tnsr_b(c, in);
  int64 lo = v_i32_add_b(a, a, 0);
  int64 hi = v_i32_add_b(b, b, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, lo);
  c[1] = 1; v_i32_st_tnsr(c, out, hi);
}
// DOUBLE-LABEL: main:
// DOUBLE-NOT:   shl
// DOUBLE:       add.i32 %V{{[0-9]+}}, [[A:%V[0-9]+]], [[A]]
// DOUBLE-NOT:   add.i32
// DOUBLE:       add.i32 carry %V{{[0-9]+}}, [[B:%V[0-9]+]], [[B]]

#elif defined(LOWZERO)
// Adding a 64-bit constant with a low word of 0: x + 0 was folded away, and
// the carry read took the carry of the add before it.
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 b = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 x = v_i32_ld_tnsr_b(c, in);
  int64 p = v_i32_add_b(x, b, 0);
  int64 lo = v_i32_add_b(a, 0, 0);
  int64 hi = v_i32_add_b(b, 1, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, p);
  c[1] = 1; v_i32_st_tnsr(c, out, lo);
  c[1] = 2; v_i32_st_tnsr(c, out, hi);
}
// LOWZERO-LABEL: main:
// LOWZERO:       add.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0
// LOWZERO-NOT:   add.i32
// LOWZERO:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1

#elif defined(SAMEHIGH)
// A 64-bit base plus two 32-bit offsets: the two carry reads are equal, and
// were merged into one (also at -O0, in SelectionDAG).
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 blo = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 bhi = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 o1 = v_i32_ld_tnsr_b(c, in);
  c[1] = 3; int64 o2 = v_i32_ld_tnsr_b(c, in);
  int64 lo1 = v_i32_add_b(blo, o1, 0);
  int64 hi1 = v_i32_add_b(bhi, 0, SW_CARRY);
  int64 lo2 = v_i32_add_b(blo, o2, 0);
  int64 hi2 = v_i32_add_b(bhi, 0, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, lo1);
  c[1] = 1; v_i32_st_tnsr(c, out, hi1);
  c[1] = 2; v_i32_st_tnsr(c, out, lo2);
  c[1] = 3; v_i32_st_tnsr(c, out, hi2);
}
// SAMEHIGH-LABEL: main:
// SAMEHIGH:       add.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
// SAMEHIGH-NOT:   add.i32
// SAMEHIGH:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0
// SAMEHIGH-NOT:   add.i32
// SAMEHIGH:       add.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
// SAMEHIGH-NOT:   add.i32
// SAMEHIGH:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0

#elif defined(DEADLOW)
// Only the carry is wanted: the low add was removed as dead (also at -O0).
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 b = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 x = v_i32_ld_tnsr_b(c, in);
  int64 p = v_i32_add_b(x, b, 0);
  v_i32_add_b(a, b, 0);
  int64 k = v_i32_add_b(x, 0, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, p);
  c[1] = 1; v_i32_st_tnsr(c, out, k);
}
// DEADLOW-LABEL: main:
// DEADLOW:       add.i32
// DEADLOW:       add.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
// DEADLOW-NOT:   add.i32
// DEADLOW:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0

#elif defined(SCALAR)
// The same on the SPU: x + x was a shift.
void main(tensor out, int a, int b) {
  int5 c = {0, 0, 0, 0, 0};
  int lo = s_i32_add(a, a, 0);
  int hi = s_i32_add(b, b, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, lo);
  c[1] = 1; v_i32_st_tnsr(c, out, hi);
}
// SCALAR-LABEL: main:
// SCALAR-NOT:   shl
// SCALAR:       add.i32 %S{{[0-9]+}}, [[A:%S[0-9]+]], [[A]]
// SCALAR-NOT:   add.i32
// SCALAR:       add.i32 carry %S{{[0-9]+}}, [[B:%S[0-9]+]], [[B]]

#elif defined(LOOP)
// The low words do not change in the loop: LICM moved the low add out of
// it, and the carry read took the carry of the add of acc.
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 b = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 acc = v_i32_ld_tnsr_b(c, in);
  c[1] = 3; int64 d = v_i32_ld_tnsr_b(c, in);
  for (int i = 0; i < 4; i++) {
    int64 lo = v_i32_add_b(a, b, 0);
    int64 hi = v_i32_add_b(acc, b, SW_CARRY);
    c[1] = i; v_i32_st_tnsr(c, out, hi);
    c[1] = 4 + i; v_i32_st_tnsr(c, out, lo);
    acc = acc + d;
  }
}
// The loop is unrolled; each carry read follows its own low add.
// LOOP-LABEL: main:
// LOOP:       add.i32 %V{{[0-9]+}}, [[A:%V[0-9]+]], [[B:%V[0-9]+]]
// LOOP-NOT:   add.i32
// LOOP:       add.i32 carry
// LOOP:       add.i32 %V{{[0-9]+}}, [[A]], [[B]]
// LOOP-NOT:   add.i32
// LOOP:       add.i32 carry
// LOOP:       add.i32 %V{{[0-9]+}}, [[A]], [[B]]
// LOOP-NOT:   add.i32
// LOOP:       add.i32 carry
// LOOP:       add.i32 %V{{[0-9]+}}, [[A]], [[B]]
// LOOP-NOT:   add.i32
// LOOP:       add.i32 carry

#elif defined(SUBDEADLOW)
// Only the carry of the low sub is wanted: it was removed as dead.
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 b = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 x = v_i32_ld_tnsr_b(c, in);
  c[1] = 3; int64 y = v_i32_ld_tnsr_b(c, in);
  int64 p = v_i32_add_b(x, b, 0);
  v_i32_sub_b(a, b, 0);
  int64 k = v_i32_add_b(x, ~y, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, p);
  c[1] = 1; v_i32_st_tnsr(c, out, k);
}
// SUBDEADLOW-LABEL: main:
// SUBDEADLOW:       sub.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
// SUBDEADLOW-NOT:   {{(add|sub)}}.i32
// SUBDEADLOW:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

#elif defined(SUBZERO)
// a - 0 was folded to a, which writes no carry (the carry must be 1).
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 x = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 y = v_i32_ld_tnsr_b(c, in);
  int64 p = v_i32_add_b(x, y, 0);
  int64 lo = v_i32_sub_b(a, 0, 0);
  int64 hi = v_i32_add_b(x, ~y, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, p);
  c[1] = 1; v_i32_st_tnsr(c, out, lo);
  c[1] = 2; v_i32_st_tnsr(c, out, hi);
}
// SUBZERO-LABEL: main:
// SUBZERO:       sub.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0
// SUBZERO-NOT:   {{(add|sub)}}.i32
// SUBZERO:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

#elif defined(SUBSAMEHIGH)
// Two subtracts with the same high words: the two high adds were merged.
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 b = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 x = v_i32_ld_tnsr_b(c, in);
  c[1] = 3; int64 y = v_i32_ld_tnsr_b(c, in);
  int64 lo1 = v_i32_sub_b(a, b, 0);
  int64 hi1 = v_i32_add_b(x, ~y, SW_CARRY);
  int64 lo2 = v_i32_sub_b(b, a, 0);
  int64 hi2 = v_i32_add_b(x, ~y, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, lo1);
  c[1] = 1; v_i32_st_tnsr(c, out, hi1);
  c[1] = 2; v_i32_st_tnsr(c, out, lo2);
  c[1] = 3; v_i32_st_tnsr(c, out, hi2);
}
// SUBSAMEHIGH-LABEL: main:
// SUBSAMEHIGH:       sub.i32 %V{{[0-9]+}}, [[A:%V[0-9]+]], [[B:%V[0-9]+]]
// SUBSAMEHIGH-NOT:   {{(add|sub)}}.i32
// SUBSAMEHIGH:       add.i32 carry
// SUBSAMEHIGH-NOT:   {{(add|sub)}}.i32
// SUBSAMEHIGH:       sub.i32 %V{{[0-9]+}}, [[B]], [[A]]
// SUBSAMEHIGH-NOT:   {{(add|sub)}}.i32
// SUBSAMEHIGH:       add.i32 carry

#elif defined(SUBSCALAR)
// The same on the SPU.
void main(tensor out, int a, int b, int x, int y) {
  int5 c = {0, 0, 0, 0, 0};
  int lo = s_i32_sub(a, b, 0);
  int hi = s_i32_add(x, ~y, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, lo);
  c[1] = 1; v_i32_st_tnsr(c, out, hi);
}
// SUBSCALAR-LABEL: main:
// SUBSCALAR:       sub.i32 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
// SUBSCALAR-NOT:   {{(add|sub)}}.i32
// SUBSCALAR:       add.i32 carry %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
#endif

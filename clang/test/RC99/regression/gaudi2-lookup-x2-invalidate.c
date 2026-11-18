// RUN: %tpc_clang -S -O0 -march=gaudi2 -DP=1 %s -o - | FileCheck %s --check-prefix=START
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DP=2 %s -o - | FileCheck %s --check-prefix=AFTER
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DP=3 %s -o - | FileCheck %s --check-prefix=BOTH
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DP=4 %s -o - | FileCheck %s --check-prefix=JOIN
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DP=5 %s -o - | FileCheck %s --check-prefix=LOOP
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DP=6 %s -o - | FileCheck %s --check-prefix=HOIST
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DP=7 %s -o - | FileCheck %s --check-prefix=RST
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=2 %s -o - | FileCheck %s --check-prefix=AFTER
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=6 %s -o - | FileCheck %s --check-prefix=HOIST
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=8 %s -o - | FileCheck %s --check-prefix=SCHED
// RUN: %tpc_clang -S -O2 -march=gaudi3 -DP=3 %s -o - | FileCheck %s --check-prefix=GAUDI3

// GAUDI2_0012: LUT cache tags may be wrong when a LOOKUP.X2 follows a plain
// LOOKUP, so a cache_invalidate lu must come between them. Like Habana's
// compiler, the fork adds one only where a plain lookup (or the kernel start)
// reaches an x2 lookup, and places it at the kernel start, right after the
// plain lookup, or right before the x2 lookup (TPCHwWaGeneral.cpp).
// lookup_1c without x2 and rst_lu do not matter. -O0 shows the placement;
// at -O2 the scheduler must keep the invalidate between the two lookups,
// which Habana's does not. gaudi3 has no such erratum.

#define LK_X2(v) v_i16_lookup_1c(v, 258, SW_BV16 | SW_X2, 0, 1, 0)
#define LK_1C(v) v_i16_lookup_1c(v, 259, SW_BV16, 0, 1, 0)
#define LK_PLAIN(v) (short128)v_f32_lookup((uint64)(v), 0, SW_BV32, 0, 1, 0)
// Make the next lookup depend on r (m is 0 at run time).
#define DEP(r) b ^= (ushort128)(r) & m

void main(tensor in, tensor out, int k, unsigned short m) {
  int5 idx = get_index_space_offset();
  ushort128 b = v_u16_ld_tnsr_b(idx, in);
  short128 r = 0;
#if P == 1
  r = LK_1C(b); DEP(r); r = LK_X2(b); DEP(r); r = LK_X2(b);
#elif P == 2
  r = LK_PLAIN(b); DEP(r); r = LK_X2(b);
#elif P == 3
  r = LK_X2(b); DEP(r); r = LK_PLAIN(b); DEP(r); r = LK_1C(b); DEP(r); r = LK_X2(b);
#elif P == 4
  if (k) { r = LK_PLAIN(b); DEP(r); }
  r = LK_X2(b);
#elif P == 5
  for (int i = 0; i < k; i++) { r = LK_X2(b); DEP(r); r = LK_PLAIN(b); DEP(r); }
#elif P == 6
  r = LK_PLAIN(b); DEP(r);
  for (int i = 0; i < k; i++) { r = LK_X2(b); DEP(r); }
#elif P == 7
  cache_invalidate(SW_RST_LU, 1, 0); r = LK_X2(b);
#elif P == 8
  // No dependency between the two lookups of a row.
  for (int i = 0; i < k; i++) {
    idx[1] = i;
    ushort128 a = v_u16_ld_tnsr_b(idx, in);
    v_i16_st_tnsr(idx, out, LK_PLAIN(a));
    v_i16_st_tnsr(idx, out, LK_X2(b));
  }
#endif
  v_i16_st_tnsr(idx, out, r);
}

// Only the kernel start reaches the x2 lookups: one invalidate, in the prologue.
// START:      event vpu 0xff
// START:      cache_invalidate lu
// START-NOT:  cache_invalidate
// START:      lookup_1c BV16 %V
// START-NOT:  cache_invalidate
// START:      lookup_1c BV16 x2
// START-NOT:  cache_invalidate
// START:      lookup_1c BV16 x2
// START-NOT:  cache_invalidate

// One plain lookup reaches it: the invalidate goes right after that lookup.
// AFTER:      main:
// AFTER-NOT:  {{lookup|cache_invalidate}}
// AFTER:      lookup BV32
// AFTER-NOT:  lookup
// AFTER:      cache_invalidate lu
// AFTER-NOT:  {{lookup|cache_invalidate}}
// AFTER:      lookup_1c BV16 x2

// BOTH:       cache_invalidate lu
// BOTH-NOT:   {{lookup|cache_invalidate}}
// BOTH:       lookup_1c BV16 x2
// BOTH-NOT:   {{lookup|cache_invalidate}}
// BOTH:       lookup BV32
// BOTH-NOT:   {{lookup|cache_invalidate}}
// BOTH:       cache_invalidate lu
// BOTH-NOT:   {{lookup|cache_invalidate}}
// BOTH:       lookup_1c BV16 %V
// BOTH-NOT:   {{lookup|cache_invalidate}}
// BOTH:       lookup_1c BV16 x2

// The start and a plain lookup reach it: right before the x2 lookup.
// JOIN:       main:
// JOIN-NOT:   cache_invalidate
// JOIN:       lookup BV32
// JOIN-NOT:   cache_invalidate
// JOIN:       .LBB0_2:
// JOIN-NOT:   lookup
// JOIN:       cache_invalidate lu
// JOIN-NOT:   lookup
// JOIN:       lookup_1c BV16 x2

// LOOP:       main:
// LOOP-NOT:   cache_invalidate
// LOOP:       .LBB0_2:
// LOOP-NOT:   lookup
// LOOP:       cache_invalidate lu
// LOOP-NOT:   {{lookup|cache_invalidate}}
// LOOP:       lookup_1c BV16 x2
// LOOP-NOT:   cache_invalidate
// LOOP:       lookup BV32

// The plain lookup before the loop is the only one: the invalidate stays out
// of the loop.
// HOIST:      lookup BV32
// HOIST-NOT:  lookup
// HOIST:      cache_invalidate lu
// HOIST-NOT:  cache_invalidate
// HOIST:      {{^}}.LBB0_{{[0-9]+}}:
// HOIST-NOT:  cache_invalidate
// HOIST:      lookup_1c BV16 x2
// HOIST-NOT:  cache_invalidate

// rst_lu does not count, and neither does its dummy lookup.
// RST:        cache_invalidate lu
// RST-NOT:    cache_invalidate lu
// RST:        cache_invalidate rst_lu
// RST:        lookup BV32 %LFSR
// RST-NOT:    cache_invalidate
// RST:        lookup_1c BV16 x2

// The scheduler used to move this invalidate after the x2 lookup.
// SCHED:      {{^}}.LBB0_{{[0-9]+}}:
// SCHED-NOT:  lookup
// SCHED:      lookup BV32
// SCHED-NOT:  lookup
// SCHED:      cache_invalidate lu
// SCHED-NOT:  lookup
// SCHED:      lookup_1c BV16 x2

// GAUDI3:     main:
// GAUDI3-NOT: cache_invalidate

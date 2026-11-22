// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck --check-prefix=FWD %s
// RUN: %tpc_clang -march=gaudi2 -O0 -S %s -o - | FileCheck --check-prefix=O0 %s
// RUN: %tpc_clang -march=gaudi2 -O2 -S -DOVERLAP %s -o - | FileCheck --check-prefix=OVERLAP %s
// RUN: %tpc_clang -march=gaudi2 -O2 -S -DREAD_LATER %s -o - | FileCheck --check-prefix=LATER %s

// At -O1 and up, as in Habana's compiler, TPCLocalMemoryForwarding keeps the
// vectors a kernel stages in VLM in registers:
// - By default, a load of a vector stored or loaded earlier in the block
//   takes it, an unconditional load needs no income (no mov of 0 to its
//   register), and the stores are deleted once nothing in the kernel reads
//   VLM.
// - At -O0 all of them stay.
// - With -DOVERLAP, a store at 0x80 overlaps the vectors at 0 and 0x100,
//   which are loaded again.
// - With -DREAD_LATER, a load at an address known only at run time can read
//   any store, so all of them stay, and its copies in the unrolled loop
//   share one load.

void main(tensor in, tensor out, int row) {
  int5 c = 0;
  for (int y = 0; y < 4; y++) {
    c[1] = y;
    v_f32_st_l_v(y * 256, v_f32_ld_tnsr_b(c, in));
  }
#if defined(OVERLAP)
  v_f32_st_l_v(128, v_f32_ld_tnsr_b(c, out));
#endif
  for (int y = 0; y < 4; y++) {
    c[1] = y;
    float64 v = v_f32_ld_l_v_b(y * 256) - v_f32_ld_l_v_b((3 - y) * 256);
#if defined(READ_LATER)
    v += v_f32_ld_l_v_b(row * 256);
#endif
    v_f32_st_tnsr(c, out, v);
  }
}

// FWD-LABEL: main:
// FWD-NOT: {{ld_l_v|st_l_v|mov.f32 %V[0-9]+, 0x0}}
// FWD: halt

// O0-LABEL: main:
// O0: st_l_v
// O0: ld_l_v
// O0: ld_l_v

// OVERLAP-LABEL: main:
// OVERLAP-DAG: st_l_v 0x0,
// OVERLAP-DAG: st_l_v 0x80,
// OVERLAP-DAG: ld_l_v %V{{[0-9]+}}, 0x0
// OVERLAP-DAG: ld_l_v %V{{[0-9]+}}, 0x100
// OVERLAP-NOT: ld_l_v %V{{[0-9]+}}, 0x200
// OVERLAP-NOT: ld_l_v %V{{[0-9]+}}, 0x300
// OVERLAP: halt

// LATER-LABEL: main:
// LATER-COUNT-4: st_l_v
// LATER: ld_l_v %V{{[0-9]+}}, %S
// LATER-NOT: ld_l_v
// LATER: halt

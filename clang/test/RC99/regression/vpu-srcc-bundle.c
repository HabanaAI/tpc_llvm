// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-asm-format=3 %s -o - | FileCheck --check-prefix=GEN3 %s
// RUN: %tpc_clang -march=greco -O2 -S -mllvm -tpc-asm-format=3 %s -o - | FileCheck --check-prefix=GEN3 %s
// RUN: %tpc_clang -march=gaudi -O2 -S -mllvm -tpc-asm-format=3 %s -o - | FileCheck --check-prefix=GEN2 %s

// SEL reads its fourth operand from the VPU SrcC field. In the 160-bit VLIW
// of goya, gaudi and gaudib that field is also the store slot's SrcC, and the
// VPU SrcD field the load slot's SrcB, so such an instruction cannot share a
// VLIW with a load or a store there. The 256-bit VLIW of greco, gaudi2 and
// doron1 has separate fields, and the later pairs' loads can issue with the
// first pair's SEL (format 3 prints a VLIW per line: load; scalar; vector;
// store).

void main(tensor a, tensor b, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 i0 = 0, i1 = 0, i2 = 0, i3 = 0;
  for (int y = start[1]; y < end[1]; y++) {
    i0[1] = i1[1] = i2[1] = i3[1] = y;
    i1[0] = 64;
    i2[0] = 128;
    i3[0] = 192;
    float64 a0 = v_f32_ld_tnsr_b(i0, a), b0 = v_f32_ld_tnsr_b(i0, b);
    float64 a1 = v_f32_ld_tnsr_b(i1, a), b1 = v_f32_ld_tnsr_b(i1, b);
    float64 a2 = v_f32_ld_tnsr_b(i2, a), b2 = v_f32_ld_tnsr_b(i2, b);
    float64 a3 = v_f32_ld_tnsr_b(i3, a), b3 = v_f32_ld_tnsr_b(i3, b);
    v_f32_st_tnsr(i0, out, v_f32_sel_less_f32_b(a0, b0, a0, b0));
    v_f32_st_tnsr(i1, out, v_f32_sel_less_f32_b(a1, b1, a1, b1));
    v_f32_st_tnsr(i2, out, v_f32_sel_less_f32_b(a2, b2, a2, b2));
    v_f32_st_tnsr(i3, out, v_f32_sel_less_f32_b(a3, b3, a3, b3));
  }
}

// GEN3: ld_tnsr {{[^;]*}}; {{[^;]*}}; sel_less.f32
// GEN2-NOT: ld_tnsr {{[^;]*}}; {{[^;]*}}; sel_less.f32
// GEN2-NOT: sel_less.f32 {{[^;]*}}; st_tnsr
// GEN2: halt

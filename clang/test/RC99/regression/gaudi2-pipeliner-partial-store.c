// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s

// The pipeliner's latency mutation looks through copies to the instruction
// that reads a value. A copy into ST_PARTIAL_REG is a MOV with its own latency,
// and the latency table has no register file for that register: it asserted.

void main(tensor in, tensor out, int n) {
  int5 c = 0;
  for (int i = 0; i < 4096; i += 64) {
    c[0] = i;
    float64 v = v_f32_ld_tnsr_b(c, in);
    v_f32_st_tnsr_partial(c, out, v * v, ((i >> 6) + n) & 63, 0);
  }
}

// CHECK: mov %ST_PARTIAL_REG, %S{{[0-9]+}}
// CHECK: st_tnsr {{.*}}, %ST_PARTIAL_REG

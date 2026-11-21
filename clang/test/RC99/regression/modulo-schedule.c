// RUN: %tpc_clang -march=gaudi2 -O2 -S -Rpass=pipeliner %s -o %t.s 2>&1 | FileCheck --check-prefix=REMARK %s
// RUN: FileCheck %s < %t.s
// RUN: grep -B2 -E '^\s+loop ' %t.s | not grep -E '^\s+jmpr'
// RUN: %tpc_clang -march=gaudi2 -O2 -S -Rpass=pipeliner -mllvm -tpc-modulo-schedule=false %s -o /dev/null 2>&1 | FileCheck --check-prefix=OFF --allow-empty %s

// The MachinePipeliner modulo-schedules the row loop (TPCModuloScheduling.cpp),
// which carries the previous row and so is not the machine unroller's. The
// IR unroller has made it 4 rows an
// iteration with a remainder loop that counts down. The MVE expander unrolls
// the pipelined kernel and keeps the loop as it was for the rows the kernel
// leaves; both become hardware loops, and the kept loop is not unrolled
// again. Its exit block came before it and its preheader jumped to it: the
// LOOP that TPCHardwareLoops put after that jump never ran.

void main(tensor a, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int rows = get_dim_size(a, 1);
  int5 c = 0;
  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    float64 prev = 0.0f;
    for (int y = 0; y < rows; y++) {
      c[1] = y;
      const float64 v = v_f32_ld_tnsr_b(c, a);
      v_f32_st_tnsr(c, out, (v - prev) * 0.5f);
      prev = v;
    }
  }
}

// REMARK: remark: Pipelined succesfully!
// OFF-NOT: Pipelined

// CHECK: loop {{%S[0-9]+}}, {{%S[0-9]+}}, 64, <
// CHECK: loop {{%S[0-9]+}}, {{%S[0-9]+}}, {{[0-9]*[02468]}}, <, [[KERNEL_END:\.LBB0_[0-9]+]]
// CHECK: [[KERNEL_END]]:
// CHECK: loop {{%S[0-9]+}}, {{%S[0-9]+}}, 4, <, [[REST_END:\.LBB0_[0-9]+]]
// CHECK-NOT: loop
// CHECK: [[REST_END]]:
// CHECK: loop {{.*}}, -1, >

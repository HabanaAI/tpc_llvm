// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-modulo-schedule=false %s -o - 2>&1 | FileCheck %s
// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-modulo-schedule=false -mllvm -tpc-pragma-chain-unroll=false %s -o - 2>&1 | FileCheck %s --check-prefix=OFF

// A loop_unroll pragma loop that carries a running max: the backend unroller
// gives each copy its own value and so refuses the loop, and the pragma kept
// the IR unroller away as well. TPCPragmaChainUnroll hands the loop to LLVM's
// unroller, which unrolls it by 4 in order: one max chain through the copies.
// (GaudiInfer's gated_norm_quant_bf16 took twice Habana's time without it.)

void main(tensor in, tensor out) {
  int5 c = 0;
  float64 acc = 0;
#pragma loop_unroll(4)
  for (int i = 0; i < 64; i++) {
    c[1] = i;
    acc = v_f32_max_b(acc, v_f32_ld_tnsr_b(c, in) * 3.0f);
  }
  c[1] = 0;
  v_f32_st_tnsr(c, out, acc);
}

// CHECK-NOT: warning
// CHECK:     loop 0, 64, 4, <,
// CHECK:     max.f32 [[A:%V[0-9]+]], [[A]], %V{{[0-9]+}}
// CHECK:     max.f32 [[A]], [[A]], %V{{[0-9]+}}
// CHECK:     max.f32 [[A]], [[A]], %V{{[0-9]+}}
// CHECK:     max.f32 [[A]], [[A]], %V{{[0-9]+}}

// OFF:       warning: machine_unroll(4) is not applied to the loop
// OFF:       loop 0, 64, 1, <,

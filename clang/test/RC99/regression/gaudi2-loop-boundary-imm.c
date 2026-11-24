// RUN: %tpc_clang -S -O2 -march=gaudi2 -DBOUND=7168 -mllvm -tpc-modulo-schedule=false %s -o - | FileCheck %s --check-prefix=REG
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DBOUND=6144 -mllvm -tpc-modulo-schedule=false %s -o - | FileCheck %s --check-prefix=IMM
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DBOUND=4224 -mllvm -tpc-modulo-schedule=false %s -o - | FileCheck %s --check-prefix=EPI
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DBOUND=7168 -mllvm -tpc-modulo-schedule=true %s -o - | FileCheck %s --check-prefix=SMS
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DBOUND=7168 -mllvm -tpc-modulo-schedule=false -mllvm -tpc-loop-boundary-to-reg=false %s -o - | FileCheck %s --check-prefix=OFF
// RUN: %tpc_clang -S -O2 -march=gaudi -DBOUND=7168 %s -o - | FileCheck %s --check-prefix=OFF

// On gaudi2 a LOOP's immediate boundary is encoded from bit 38, so its bits
// 12:7 fall on the VPU slot's opcode field, and the TPC stops with
// tpc_illegal_instruction when they are 0x21 or 0x38-0x3C (gaudi2 card;
// the simulator runs it). Such a boundary goes to the LOOP in a register.
// 7168 = 0x1C00 gives 0x38; 6144 gives 0x30 and stays an immediate. With
// 4224 the machine unroller's epilogue loop gets the boundary 0x1080, which
// gives 0x21. The pipeliner (SMS) makes another loop with the same boundary.

void main(tensor in, tensor out) {
  int5 c = 0;
  for (int x = 0; x < BOUND; x += 64) {
    c[0] = x;
    v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, in) * 2.0f);
  }
}

// REG:      mov.i32 [[B:%S[0-9]+]], 0x1c00
// REG:      loop 0, [[B]], 256, <

// IMM:      loop 0, 6144, 256, <

// EPI:      loop 0, 4096, 256, <
// EPI:      mov.i32 [[B:%S[0-9]+]], 0x1080
// EPI:      loop %S{{[0-9]+}}, [[B]], 64, <

// SMS:      mov.i32 [[B:%S[0-9]+]], 0x1c00
// SMS:      loop %S{{[0-9]+}}, [[B]], 64, <

// OFF:      loop 0, 7168, 256, <

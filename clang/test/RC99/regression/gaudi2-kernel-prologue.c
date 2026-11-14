// RUN: %tpc_clang -S -O2 -march=gaudi2 %s -o - | FileCheck %s
// RUN: %tpc_clang -S -O0 -march=gaudi2 %s -o - | FileCheck %s --check-prefix=O0
// RUN: %tpc_clang -S -O2 -march=gaudi2 -long-irf %s -o - | FileCheck %s --check-prefix=LIRF
// RUN: %tpc_clang -S -O2 -march=gaudi2 -mllvm -tpc-insert-event-instruction=false -mllvm -tpc-prologue-kernel-config=false %s -o - | FileCheck %s --check-prefix=OFF
// RUN: %tpc_clang -S -O2 -march=gaudi %s -o - | FileCheck %s --check-prefix=OFF
// RUN: %tpc_clang -c -O2 -march=gaudi2 %s -o %t.o && %metadata_disasm --mcpu=gaudi2 %t.o | FileCheck %s --check-prefix=MMIO

// Habana's compiler starts every gaudi2 kernel with this prologue, each
// instruction in a VLIW of its own (TPCPrologueMMIO.cpp). It writes
// KERNEL_CONFIG (0x538) only at -O1 and above, into the first scalar register
// after the arguments, and starts the kernel 5 cycles after the write. The
// write sets bit 24 (32-bit IRF arithmetic), or clears it with -long-irf.

void main(tensor in, tensor out, int a, int b) {
  int5 c = get_index_space_offset();
  int64 v = v_i32_ld_tnsr_b(c, in);
  v_i32_st_tnsr(c, out, v + a + b);
}

// CHECK:      main:
// CHECK-NEXT: // %bb.0:
// CHECK-NEXT: event vpu 0xff
// CHECK-NEXT: cache_invalidate lu
// CHECK-NEXT: ld_l mmio %S2, 0x538
// CHECK-NEXT: NOP
// CHECK-NEXT: NOP
// CHECK-NEXT: NOP
// CHECK-NEXT: or.i32 %S2, %S2, 0x1000000
// CHECK-COUNT-6: NOP
// CHECK-NEXT: st_l mmio 0x538, %S2
// CHECK-COUNT-4: NOP
// CHECK-NEXT: {

// LIRF:      ld_l mmio %S2, 0x538
// LIRF-NEXT: NOP
// LIRF-NEXT: NOP
// LIRF-NEXT: NOP
// LIRF-NEXT: and.i32 %S2, %S2, 0xfeffffff
// LIRF-COUNT-6: NOP
// LIRF-NEXT: st_l mmio 0x538, %S2

// O0:      main:
// O0-COUNT-8: nop
// O0-NEXT: event vpu 0xff
// O0-COUNT-8: nop
// O0-NEXT: cache_invalidate lu
// O0-NOT:  0x538

// OFF:     main:
// OFF-NOT: event vpu
// OFF-NOT: 0x538

// MMIO: mmioUsed: DB 1

// RUN: %clang_cc1 -emit-obj -triple tpc-none-none -std=rc99 -O1 %s -o %t.o
// RUN: llvm-objdump -s -j .text %t.o | FileCheck %s

void main(int x) {
  register int result __asm__("s11");
  register int source __asm__("s20");
  __asm volatile ("nop; mov.f32 %0, %1" :"=s" (result): "s" (source):);
}

// 0-5     SPU_OPCODE        001000 (MOV)
// 6-12    SPU_SRC_A         010100 (S20)
// 20-26   SPU_DEST          001011 (S11)
// 27-30   SPU_OPERANDS_TYPE 0000   (FP32)
// 43-48   VPU_OPCODE        111111 (NOP)
// 119-123 LOAD_OPCODE       11111  (NOP)
// 140-144 STORE_OPCODE      11111  (NOP)
// 
//  0  - 31  00001000 00000101 10110000 00000000
// 32  - 63  00000000 11111000 00000001 00000000
// 64  - 95  00000000 00000000 00000000 00000000
// 96  - 127 00000000 00000000 10000000 00001111
// 128 - 159 00000000 11110000 00000001 00000000
// 160 - 191 00000000 00000000 00000000 00000000
//
// CHECK:  0000 3f000000 00f80100 00000000 0000800f
// CHECK:  0010 00500100 00000000 00000000 00000000


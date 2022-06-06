// RUN: %clang_cc1 -emit-obj -triple tpc-none-none -std=rc99 -O1 %s -o %t.o
// RUN: %llvm-objdump --triple tpc -s -j .text %t.o | FileCheck %s

void main(int x) {
  register float64 result __asm__("v3");
  register float64 source __asm__("v6");
  __asm volatile ("nop; nop; mov.f32 %0, %1" :"=S" (result): "s" (source):);
}

// 0-5     SPU_OPCODE        111111   (NOP)
// 43-48   VPU_OPCODE        001000   (MOV)
// 48-55   VPU_SRC_A         00000110 (V6)
// 80-87   VPU_DEST          00000011 (V3)
// 93-96   VPU_OPERAND_TYPE  0000     (FP32)
// 115-119 LOAD_OPCODE       11111    (NOP)
// 136-140 STORE_OPCODE      11111    (NOP)
//
//                  0        8        1        2
//                                    6        4
//  0  - 31  00111111 00000000 00000000 00000000
// 32  - 63  00000000 00100000 00001100 00000000
// 64  - 95  00000000 00000000 00000011 00000000
// 96  - 127 00000000 00000000 10000000 00001111
// 128 - 159 00000000 11110000 00000001 00000000
// 160 - 191 00000000 00000000 00000000 00000000
//
// CHECK:  0000 3f000000 00f80100 00000000 0000800f
// CHECK:  0010 00500100 00000000 00000000 00000000


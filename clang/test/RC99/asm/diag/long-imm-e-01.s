// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 %s 2>&1 | FileCheck --check-prefix=GAUDI2 %s
// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu gaudi %s 2>&1 | FileCheck --check-prefix=GAUDI %s

// A VLIW has one immediate field, for the immediates that do not fit in their
// instruction (on Gaudi2, those outside 0..15, or -8..7 when signed; before
// Gaudi2, all of them) and for a jump target. Two different values there were
// accepted, and one of them was encoded for both.

.L0:
NOP; ADD.I32 b00001 I5, 0x80, I5; NOP; SET_INDX I7, b00001, 0x400
// GAUDI2: long-imm-e-01.s:[[@LINE-1]]:1: error: two different long immediates in one VLIW (0x80 and 0x400): it has one immediate field
// GAUDI:  long-imm-e-01.s:[[@LINE-2]]:1: error: two different long immediates in one VLIW (0x80 and 0x400): it has one immediate field

// The same value twice uses the field once.
NOP; ADD.I32 b00001 I5, 0x400, I5; NOP; SET_INDX I7, b00001, 0x400
// GAUDI2-NOT: long-imm-e-01.s:[[@LINE-1]]:
// GAUDI-NOT:  long-imm-e-01.s:[[@LINE-2]]:

// Gaudi2 encodes 1 and 2 in the instructions; Gaudi has only the field.
NOP; ADD.I32 b00001 I5, 0x1, I5; NOP; SET_INDX I7, b00001, 0x2
// GAUDI2-NOT: long-imm-e-01.s:[[@LINE-1]]:
// GAUDI:      long-imm-e-01.s:[[@LINE-2]]:1: error: two different long immediates in one VLIW (0x1 and 0x2): it has one immediate field

NOP; JMPR .L0, SP1; NOP; SET_INDX I7, b00001, 0x400
// GAUDI2: long-imm-e-01.s:[[@LINE-1]]:1: error: a jump target and another long immediate in one VLIW: it has one immediate field
// GAUDI:  long-imm-e-01.s:[[@LINE-2]]:1: error: a jump target and another long immediate in one VLIW: it has one immediate field

NOP; JMPR .L0, SP1; NOP; SET_INDX I7, b00001, 0x3
// GAUDI2-NOT: long-imm-e-01.s:[[@LINE-1]]:
// GAUDI:      long-imm-e-01.s:[[@LINE-2]]:1: error: a jump target and another long immediate in one VLIW: it has one immediate field

NOP; HALT; HALT; NOP

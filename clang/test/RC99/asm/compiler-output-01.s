// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi2 %t.o | FileCheck %s
// RUN: llvm-readelf -S -x .tpc_metadata %t.o | FileCheck --check-prefix=SEC %s

// Assembly as the compiler prints it: registers with '%', READ for a MOV from
// the lane id, ".tpc_compiler" marked "a", and .tpc_metadata as .asciz bytes.
// The parser rejected the first three. For the last, it wrote a default header
// and left the bytes in the previous section.

main:
{
	nop
	mov.i32  %S2, %S0, !%SP1
	READ	%V0, V_LANE_ID_32
	nop
}
{
	nop
	halt
	halt
	nop
}

	.section	.tpc_compiler,"a",@progbits
	.ascii	"-O2"

	.section	.tpc_metadata,"aw",@progbits
	.globl	tpc_metadata
tpc_metadata:
	.asciz	"\024\000\000\000\000\000\000\001\004\000\000\000\001\000\001\000\000\000\256\274\270\013\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

// CHECK:      nop; mov.i32 S2, S0, !SP1; mov V0, V_LANE_ID_32; nop
// CHECK-NEXT: nop; halt; halt; nop

// SEC:      .tpc_compiler PROGBITS
// SEC:      .text         PROGBITS {{[0-9a-f]+}} {{[0-9a-f]+}} 000040
// SEC:      .tpc_metadata PROGBITS {{[0-9a-f]+}} {{[0-9a-f]+}} 000106
// SEC:      Hex dump of section '.tpc_metadata':
// SEC-NEXT: 0x00000000 14000000 00000001 04000000 01000100

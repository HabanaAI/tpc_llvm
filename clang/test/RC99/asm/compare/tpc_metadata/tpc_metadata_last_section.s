// RUN: %tpc_clang -c -x assembler -no-instr-compress -march=gaudi %s -o %t.o
// RUN: %metadata_disasm %t.o | FileCheck %s

// CHECK: .section .tpc_metadata
// CHECK:	version: DD 11

main:
               	nop; 	nop; 	nop; 	nop
               	nop; 	nop; 	nop; 	nop
               	nop; 	halt; 	halt; 	nop
               	nop; 	nop; 	nop; 	nop
               	nop; 	nop; 	nop; 	nop
               	nop; 	nop; 	nop; 	nop

.section .tpc_metadata
	version: DD 11
	specialFunctionUsed: DB 0
	printfUsed: DB 0
	lockUnLock: DB 0
	mmioUsed: DB 1
	march: DW 2
	paramsNum: DB 1
	printfTensorID: DB 0
	numberOfThreads: DB 1
	directMMIOAccess: DB 0
	scalarLd[1]: DB 1

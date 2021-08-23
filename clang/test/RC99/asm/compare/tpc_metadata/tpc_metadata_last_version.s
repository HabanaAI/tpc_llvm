// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %metadata_disasm %t.o | FileCheck %s

.section .TPC_METADATA
version:				DD	9
specialFunctionUsed:	DB	1
printfUsed:				DB	0
scalarld:				DH	0101001011100011
LockUnLock:				DB	1
rmwStore:				DH	1010000100111110
march:					DW	1
mmioUsed:				DB	1
paramsNum:				DB	3
printfTensorID:			DB	3 

// CHECK: .section .tpc_metadata
// CHECK:	version: DD 9
// CHECK:	specialFunctionUsed: DB 1
// CHECK:	printfUsed: DB 0
// CHECK:	lockUnLock: DB 1
// CHECK:	scalarLd[1]: DB 1
// CHECK:	scalarLd[3]: DB 1
// CHECK:	scalarLd[6]: DB 1
// CHECK:	scalarLd[8]: DB 1
// CHECK:	scalarLd[9]: DB 1
// CHECK:	scalarLd[10]: DB 1
// CHECK:	scalarLd[14]: DB 1
// CHECK:	scalarLd[15]: DB 1
// CHECK:	rmwStore[0]: DB 1
// CHECK:	rmwStore[2]: DB 1
// CHECK:	rmwStore[7]: DB 1
// CHECK:	rmwStore[10]: DB 1
// CHECK:	rmwStore[11]: DB 1
// CHECK:	rmwStore[12]: DB 1
// CHECK:	rmwStore[13]: DB 1
// CHECK:	rmwStore[14]: DB 1
// CHECK:	march: DW 1
// CHECK:	mmioUsed: DB 1
// CHECK:	paramsNum: DB 3
// CHECK:	printfTensorID: DB 3

NOP;

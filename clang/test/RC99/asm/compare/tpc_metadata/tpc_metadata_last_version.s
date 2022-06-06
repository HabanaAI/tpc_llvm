// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %metadata_disasm %t.o | FileCheck %s

.section .TPC_METADATA
version:				DD	12
specialFunctionUsed:	DB	1
printfUsed:				DB	0
scalarld:				DH	0101001011100011
LockUnLock:				DB	1
rmwStore:				DH	1010000100111110
march:					DW	1
mmioUsed:				DB	1
paramsNum:				DB	3
printfTensorID:			DB	3
numberOfThreads:		DB	4
directMMIOAccess:		DB	1
dnorm:  DB 1

// CHECK: .section .tpc_metadata
// CHECK-NEXT:	version: DD 12
// CHECK-NEXT:	specialFunctionUsed: DB 1
// CHECK-NEXT:	printfUsed: DB 0
// CHECK-NEXT:	lockUnLock: DB 1
// CHECK-NEXT:	mmioUsed: DB 1
// CHECK-NEXT:	march: DW 1
// CHECK-NEXT:	paramsNum: DB 3
// CHECK-NEXT:	printfTensorID: DB 3
// CHECK-NEXT:	numberOfThreads: DB 4
// CHECK-NEXT:	directMMIOAccess: DB 1
// CHECK-NEXT:  dnorm: DB 1
// CHECK-NEXT:	scalarLd[1]: DB 1
// CHECK-NEXT:	scalarLd[3]: DB 1
// CHECK-NEXT:	scalarLd[6]: DB 1
// CHECK-NEXT:	scalarLd[8]: DB 1
// CHECK-NEXT:	scalarLd[9]: DB 1
// CHECK-NEXT:	scalarLd[10]: DB 1
// CHECK-NEXT:	scalarLd[14]: DB 1
// CHECK-NEXT:	scalarLd[15]: DB 1
// CHECK-NEXT:	rmwStore[0]: DB 1
// CHECK-NEXT:	rmwStore[2]: DB 1
// CHECK-NEXT:	rmwStore[7]: DB 1
// CHECK-NEXT:	rmwStore[10]: DB 1
// CHECK-NEXT:	rmwStore[11]: DB 1
// CHECK-NEXT:	rmwStore[12]: DB 1
// CHECK-NEXT:	rmwStore[13]: DB 1
// CHECK-NEXT:	rmwStore[14]: DB 1

NOP;

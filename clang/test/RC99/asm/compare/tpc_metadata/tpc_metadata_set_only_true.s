// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %metadata_disasm %t.o | FileCheck %s

.tpc_metadata
version:				DD	6
specialFunctionUsed:	DB	1
printfUsed:				DB	0
lockUnLock:				DB	1
scalarLd[1]:			DB	1
scalarLd[3]:			DB	1
scalarLd[6]:			DB	1
scalarLd[8]:			DB	1
scalarLd[9]:			DB	1
scalarLd[10]:			DB	1
scalarLd[14]:			DB	1
scalarLd[15]:			DB	1
rmwStore[0]:			DB	1
rmwStore[2]:			DB	1
rmwStore[7]:			DB	1
rmwStore[10]:			DB	1
rmwStore[11]:			DB	1
rmwStore[12]:			DB	1
rmwStore[13]:			DB	1
rmwStore[14]:			DB	1
march:					DW	1
mmioUsed:				DB	0

// CHECK: .section .tpc_metadata
// CHECK:	version: DD 6
// CHECK:	specialFunctionUsed: DB 1
// CHECK:	printfUsed: DB 0
// CHECK:	lockUnLock: DB 1
// CHECK:	mmioUsed: DB	0
// CHECK:	march: DW 1
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

NOP;

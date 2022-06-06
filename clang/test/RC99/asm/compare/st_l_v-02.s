// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

NOP; NOP; NOP; ST_L_V S0, V16
NOP; NOP; NOP; ST_L_V S1, V17
NOP; NOP; NOP; ST_L_V S2, V18
NOP; NOP; NOP; ST_L_V S3, V19
NOP; NOP; NOP; ST_L_V S4, V20
NOP; NOP; NOP; ST_L_V S5, V21
NOP; NOP; NOP; ST_L_V S6, V22
NOP; NOP; NOP; ST_L_V S7, V23

// CHECK: nop;    nop;    nop;    st_l_v S0, V16
// CHECK: nop;    nop;    nop;    st_l_v S1, V17
// CHECK: nop;    nop;    nop;    st_l_v S2, V18
// CHECK: nop;    nop;    nop;    st_l_v S3, V19
// CHECK: nop;    nop;    nop;    st_l_v S4, V20
// CHECK: nop;    nop;    nop;    st_l_v S5, V21
// CHECK: nop;    nop;    nop;    st_l_v S6, V22
// CHECK: nop;    nop;    nop;    st_l_v S7, V23


NOP; NOP; NOP; ST_L_V S35, V16
NOP; NOP; NOP; ST_L_V S34, V17, SP1
NOP; NOP; NOP; ST_L_V S33, V18, SP2
NOP; NOP; NOP; ST_L_V S32, V19, SP3
NOP; NOP; NOP; ST_L_V S31, V20, SP4
NOP; NOP; NOP; ST_L_V S30, V21, SP5
NOP; NOP; NOP; ST_L_V S29, V22, SP6
NOP; NOP; NOP; ST_L_V S28, V23, SP7

// CHECK: nop;    nop;    nop;    st_l_v S35, V16
// CHECK: nop;    nop;    nop;    st_l_v S34, V17, SP1
// CHECK: nop;    nop;    nop;    st_l_v S33, V18, SP2
// CHECK: nop;    nop;    nop;    st_l_v S32, V19, SP3
// CHECK: nop;    nop;    nop;    st_l_v S31, V20, SP4
// CHECK: nop;    nop;    nop;    st_l_v S30, V21, SP5
// CHECK: nop;    nop;    nop;    st_l_v S29, V22, SP6
// CHECK: nop;    nop;    nop;    st_l_v S28, V23, SP7


NOP; NOP; NOP; ST_L_V S0, V16, !SP0
NOP; NOP; NOP; ST_L_V S1, V17, !SP1
NOP; NOP; NOP; ST_L_V S2, V18, !SP2
NOP; NOP; NOP; ST_L_V S3, V19, !SP3
NOP; NOP; NOP; ST_L_V S4, V20, !SP4
NOP; NOP; NOP; ST_L_V S5, V21, !SP5
NOP; NOP; NOP; ST_L_V S6, V22, !SP6
NOP; NOP; NOP; ST_L_V S7, V23, !SP7

// CHECK: nop;    nop;    nop;    st_l_v S0, V16, !SP0
// CHECK: nop;    nop;    nop;    st_l_v S1, V17, !SP1
// CHECK: nop;    nop;    nop;    st_l_v S2, V18, !SP2
// CHECK: nop;    nop;    nop;    st_l_v S3, V19, !SP3
// CHECK: nop;    nop;    nop;    st_l_v S4, V20, !SP4
// CHECK: nop;    nop;    nop;    st_l_v S5, V21, !SP5
// CHECK: nop;    nop;    nop;    st_l_v S6, V22, !SP6
// CHECK: nop;    nop;    nop;    st_l_v S7, V23, !SP7


NOP; NOP; NOP; ST_L_V S35, VP6
NOP; NOP; NOP; ST_L_V S34, VP7
NOP; NOP; NOP; ST_L_V S33, VP8
NOP; NOP; NOP; ST_L_V S32, VP9
NOP; NOP; NOP; ST_L_V S31, VP0
NOP; NOP; NOP; ST_L_V S30, VP1
NOP; NOP; NOP; ST_L_V S29, VP2
NOP; NOP; NOP; ST_L_V S28, VP3

// CHECK: nop;    nop;    nop;    st_l_v S35, VP6
// CHECK: nop;    nop;    nop;    st_l_v S34, VP7
// CHECK: nop;    nop;    nop;    st_l_v S33, VP8
// CHECK: nop;    nop;    nop;    st_l_v S32, VP9
// CHECK: nop;    nop;    nop;    st_l_v S31
// CHECK: nop;    nop;    nop;    st_l_v S30, VP1
// CHECK: nop;    nop;    nop;    st_l_v S29, VP2
// CHECK: nop;    nop;    nop;    st_l_v S28, VP3


NOP; NOP; NOP; ST_L_V S0, VP6
NOP; NOP; NOP; ST_L_V S1, VP7, SP1
NOP; NOP; NOP; ST_L_V S2, VP8, SP2
NOP; NOP; NOP; ST_L_V S3, VP9, SP3
NOP; NOP; NOP; ST_L_V S4, VP0, SP4
NOP; NOP; NOP; ST_L_V S5, VP1, SP5
NOP; NOP; NOP; ST_L_V S6, VP2, SP6
NOP; NOP; NOP; ST_L_V S7, VP3, SP7

// CHECK: nop;    nop;    nop;    st_l_v S0, VP6
// CHECK: nop;    nop;    nop;    st_l_v S1, VP7, SP1
// CHECK: nop;    nop;    nop;    st_l_v S2, VP8, SP2
// CHECK: nop;    nop;    nop;    st_l_v S3, VP9, SP3
// CHECK: nop;    nop;    nop;    st_l_v S4, VP0, SP4
// CHECK: nop;    nop;    nop;    st_l_v S5, VP1, SP5
// CHECK: nop;    nop;    nop;    st_l_v S6, VP2, SP6
// CHECK: nop;    nop;    nop;    st_l_v S7, VP3, SP7


NOP; NOP; NOP; ST_L_V S35, VP6, !SP0
NOP; NOP; NOP; ST_L_V S34, VP7, !SP1
NOP; NOP; NOP; ST_L_V S33, VP8, !SP2
NOP; NOP; NOP; ST_L_V S32, VP9, !SP3
NOP; NOP; NOP; ST_L_V S31, VP0, !SP4
NOP; NOP; NOP; ST_L_V S30, VP1, !SP5
NOP; NOP; NOP; ST_L_V S29, VP2, !SP6
NOP; NOP; NOP; ST_L_V S28, VP3, !SP7

// CHECK: nop;    nop;    nop;    st_l_v S35, VP6, !SP0
// CHECK: nop;    nop;    nop;    st_l_v S34, VP7, !SP1
// CHECK: nop;    nop;    nop;    st_l_v S33, VP8, !SP2
// CHECK: nop;    nop;    nop;    st_l_v S32, VP9, !SP3
// CHECK: nop;    nop;    nop;    st_l_v S31, VP0, !SP4
// CHECK: nop;    nop;    nop;    st_l_v S30, VP1, !SP5
// CHECK: nop;    nop;    nop;    st_l_v S29, VP2, !SP6
// CHECK: nop;    nop;    nop;    st_l_v S28, VP3, !SP7


NOP; NOP; NOP; ST_L_V 123, V16
NOP; NOP; NOP; ST_L_V 123, V17
NOP; NOP; NOP; ST_L_V 123, V18
NOP; NOP; NOP; ST_L_V 123, V19
NOP; NOP; NOP; ST_L_V 123, V20
NOP; NOP; NOP; ST_L_V 123, V21
NOP; NOP; NOP; ST_L_V 123, V22
NOP; NOP; NOP; ST_L_V 123, V23

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V16
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V17
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V18
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V19
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V20
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V21
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V22
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V23


NOP; NOP; NOP; ST_L_V 123, V16
NOP; NOP; NOP; ST_L_V 123, V17, SP1
NOP; NOP; NOP; ST_L_V 123, V18, SP2
NOP; NOP; NOP; ST_L_V 123, V19, SP3
NOP; NOP; NOP; ST_L_V 123, V20, SP4
NOP; NOP; NOP; ST_L_V 123, V21, SP5
NOP; NOP; NOP; ST_L_V 123, V22, SP6
NOP; NOP; NOP; ST_L_V 123, V23, SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V16
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V17, SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V18, SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V19, SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V20, SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V21, SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V22, SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V23, SP7


NOP; NOP; NOP; ST_L_V 123, V16, !SP0
NOP; NOP; NOP; ST_L_V 123, V17, !SP1
NOP; NOP; NOP; ST_L_V 123, V18, !SP2
NOP; NOP; NOP; ST_L_V 123, V19, !SP3
NOP; NOP; NOP; ST_L_V 123, V20, !SP4
NOP; NOP; NOP; ST_L_V 123, V21, !SP5
NOP; NOP; NOP; ST_L_V 123, V22, !SP6
NOP; NOP; NOP; ST_L_V 123, V23, !SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V16, !SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V17, !SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V18, !SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V19, !SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V20, !SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V21, !SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V22, !SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, V23, !SP7


NOP; NOP; NOP; ST_L_V 123, VP6
NOP; NOP; NOP; ST_L_V 123, VP7
NOP; NOP; NOP; ST_L_V 123, VP8
NOP; NOP; NOP; ST_L_V 123, VP9
NOP; NOP; NOP; ST_L_V 123, VP0
NOP; NOP; NOP; ST_L_V 123, VP1
NOP; NOP; NOP; ST_L_V 123, VP2
NOP; NOP; NOP; ST_L_V 123, VP3

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP7
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP8
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP9
// CHECK: nop;    nop;    nop;    st_l_v 0x7b
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP3


NOP; NOP; NOP; ST_L_V 123, VP6
NOP; NOP; NOP; ST_L_V 123, VP7, SP1
NOP; NOP; NOP; ST_L_V 123, VP8, SP2
NOP; NOP; NOP; ST_L_V 123, VP9, SP3
NOP; NOP; NOP; ST_L_V 123, VP0, SP4
NOP; NOP; NOP; ST_L_V 123, VP1, SP5
NOP; NOP; NOP; ST_L_V 123, VP2, SP6
NOP; NOP; NOP; ST_L_V 123, VP3, SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP7, SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP8, SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP9, SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP0, SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP1, SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP2, SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP3, SP7


NOP; NOP; NOP; ST_L_V 123, VP6, !SP0
NOP; NOP; NOP; ST_L_V 123, VP7, !SP1
NOP; NOP; NOP; ST_L_V 123, VP8, !SP2
NOP; NOP; NOP; ST_L_V 123, VP9, !SP3
NOP; NOP; NOP; ST_L_V 123, VP0, !SP4
NOP; NOP; NOP; ST_L_V 123, VP1, !SP5
NOP; NOP; NOP; ST_L_V 123, VP2, !SP6
NOP; NOP; NOP; ST_L_V 123, VP3, !SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP6, !SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP7, !SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP8, !SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP9, !SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP0, !SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP1, !SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP2, !SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, VP3, !SP7

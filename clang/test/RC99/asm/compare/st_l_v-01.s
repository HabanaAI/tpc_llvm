// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; NOP; ST_L_V S0, S35, V16
NOP; NOP; NOP; ST_L_V S1, S34, V17
NOP; NOP; NOP; ST_L_V S2, S33, V18
NOP; NOP; NOP; ST_L_V S3, S32, V19
NOP; NOP; NOP; ST_L_V S4, S31, V20
NOP; NOP; NOP; ST_L_V S5, S30, V21
NOP; NOP; NOP; ST_L_V S6, S29, V22
NOP; NOP; NOP; ST_L_V S7, S28, V23

// CHECK: nop;    nop;    nop;    st_l_v S0, S35, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v S1, S34, V17, SP0
// CHECK: nop;    nop;    nop;    st_l_v S2, S33, V18, SP0
// CHECK: nop;    nop;    nop;    st_l_v S3, S32, V19, SP0
// CHECK: nop;    nop;    nop;    st_l_v S4, S31, V20, SP0
// CHECK: nop;    nop;    nop;    st_l_v S5, S30, V21, SP0
// CHECK: nop;    nop;    nop;    st_l_v S6, S29, V22, SP0
// CHECK: nop;    nop;    nop;    st_l_v S7, S28, V23, SP0


NOP; NOP; NOP; ST_L_V S0, S35, V16, SP0
NOP; NOP; NOP; ST_L_V S1, S34, V17, SP1
NOP; NOP; NOP; ST_L_V S2, S33, V18, SP2
NOP; NOP; NOP; ST_L_V S3, S32, V19, SP3
NOP; NOP; NOP; ST_L_V S4, S31, V20, SP4
NOP; NOP; NOP; ST_L_V S5, S30, V21, SP5
NOP; NOP; NOP; ST_L_V S6, S29, V22, SP6
NOP; NOP; NOP; ST_L_V S7, S28, V23, SP7

// CHECK: nop;    nop;    nop;    st_l_v S0, S35, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v S1, S34, V17, SP1
// CHECK: nop;    nop;    nop;    st_l_v S2, S33, V18, SP2
// CHECK: nop;    nop;    nop;    st_l_v S3, S32, V19, SP3
// CHECK: nop;    nop;    nop;    st_l_v S4, S31, V20, SP4
// CHECK: nop;    nop;    nop;    st_l_v S5, S30, V21, SP5
// CHECK: nop;    nop;    nop;    st_l_v S6, S29, V22, SP6
// CHECK: nop;    nop;    nop;    st_l_v S7, S28, V23, SP7


NOP; NOP; NOP; ST_L_V S0, S35, V16, !SP0
NOP; NOP; NOP; ST_L_V S1, S34, V17, !SP1
NOP; NOP; NOP; ST_L_V S2, S33, V18, !SP2
NOP; NOP; NOP; ST_L_V S3, S32, V19, !SP3
NOP; NOP; NOP; ST_L_V S4, S31, V20, !SP4
NOP; NOP; NOP; ST_L_V S5, S30, V21, !SP5
NOP; NOP; NOP; ST_L_V S6, S29, V22, !SP6
NOP; NOP; NOP; ST_L_V S7, S28, V23, !SP7

// CHECK: nop;    nop;    nop;    st_l_v S0, S35, V16, !SP0
// CHECK: nop;    nop;    nop;    st_l_v S1, S34, V17, !SP1
// CHECK: nop;    nop;    nop;    st_l_v S2, S33, V18, !SP2
// CHECK: nop;    nop;    nop;    st_l_v S3, S32, V19, !SP3
// CHECK: nop;    nop;    nop;    st_l_v S4, S31, V20, !SP4
// CHECK: nop;    nop;    nop;    st_l_v S5, S30, V21, !SP5
// CHECK: nop;    nop;    nop;    st_l_v S6, S29, V22, !SP6
// CHECK: nop;    nop;    nop;    st_l_v S7, S28, V23, !SP7


NOP; NOP; NOP; ST_L_V S0, S35, VP6
NOP; NOP; NOP; ST_L_V S1, S34, VP7
NOP; NOP; NOP; ST_L_V S2, S33, VP8
NOP; NOP; NOP; ST_L_V S3, S32, VP9
NOP; NOP; NOP; ST_L_V S4, S31, VP0
NOP; NOP; NOP; ST_L_V S5, S30, VP1
NOP; NOP; NOP; ST_L_V S6, S29, VP2
NOP; NOP; NOP; ST_L_V S7, S28, VP3

// CHECK: nop;    nop;    nop;    st_l_v S0, S35, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v S1, S34, VP7, SP0
// CHECK: nop;    nop;    nop;    st_l_v S2, S33, VP8, SP0
// CHECK: nop;    nop;    nop;    st_l_v S3, S32, VP9, SP0
// CHECK: nop;    nop;    nop;    st_l_v S4, S31, VP0, SP0
// CHECK: nop;    nop;    nop;    st_l_v S5, S30, VP1, SP0
// CHECK: nop;    nop;    nop;    st_l_v S6, S29, VP2, SP0
// CHECK: nop;    nop;    nop;    st_l_v S7, S28, VP3, SP0


NOP; NOP; NOP; ST_L_V S0, S35, VP6, SP0
NOP; NOP; NOP; ST_L_V S1, S34, VP7, SP1
NOP; NOP; NOP; ST_L_V S2, S33, VP8, SP2
NOP; NOP; NOP; ST_L_V S3, S32, VP9, SP3
NOP; NOP; NOP; ST_L_V S4, S31, VP0, SP4
NOP; NOP; NOP; ST_L_V S5, S30, VP1, SP5
NOP; NOP; NOP; ST_L_V S6, S29, VP2, SP6
NOP; NOP; NOP; ST_L_V S7, S28, VP3, SP7

// CHECK: nop;    nop;    nop;    st_l_v S0, S35, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v S1, S34, VP7, SP1
// CHECK: nop;    nop;    nop;    st_l_v S2, S33, VP8, SP2
// CHECK: nop;    nop;    nop;    st_l_v S3, S32, VP9, SP3
// CHECK: nop;    nop;    nop;    st_l_v S4, S31, VP0, SP4
// CHECK: nop;    nop;    nop;    st_l_v S5, S30, VP1, SP5
// CHECK: nop;    nop;    nop;    st_l_v S6, S29, VP2, SP6
// CHECK: nop;    nop;    nop;    st_l_v S7, S28, VP3, SP7


NOP; NOP; NOP; ST_L_V S0, S35, VP6, !SP0
NOP; NOP; NOP; ST_L_V S1, S34, VP7, !SP1
NOP; NOP; NOP; ST_L_V S2, S33, VP8, !SP2
NOP; NOP; NOP; ST_L_V S3, S32, VP9, !SP3
NOP; NOP; NOP; ST_L_V S4, S31, VP0, !SP4
NOP; NOP; NOP; ST_L_V S5, S30, VP1, !SP5
NOP; NOP; NOP; ST_L_V S6, S29, VP2, !SP6
NOP; NOP; NOP; ST_L_V S7, S28, VP3, !SP7

// CHECK: nop;    nop;    nop;    st_l_v S0, S35, VP6, !SP0
// CHECK: nop;    nop;    nop;    st_l_v S1, S34, VP7, !SP1
// CHECK: nop;    nop;    nop;    st_l_v S2, S33, VP8, !SP2
// CHECK: nop;    nop;    nop;    st_l_v S3, S32, VP9, !SP3
// CHECK: nop;    nop;    nop;    st_l_v S4, S31, VP0, !SP4
// CHECK: nop;    nop;    nop;    st_l_v S5, S30, VP1, !SP5
// CHECK: nop;    nop;    nop;    st_l_v S6, S29, VP2, !SP6
// CHECK: nop;    nop;    nop;    st_l_v S7, S28, VP3, !SP7


NOP; NOP; NOP; ST_L_V 123, S0, V16
NOP; NOP; NOP; ST_L_V 123, S1, V17
NOP; NOP; NOP; ST_L_V 123, S2, V18
NOP; NOP; NOP; ST_L_V 123, S3, V19
NOP; NOP; NOP; ST_L_V 123, S4, V20
NOP; NOP; NOP; ST_L_V 123, S5, V21
NOP; NOP; NOP; ST_L_V 123, S6, V22
NOP; NOP; NOP; ST_L_V 123, S7, V23

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S0, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S1, V17, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S2, V18, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S3, V19, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S4, V20, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S5, V21, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S6, V22, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S7, V23, SP0


NOP; NOP; NOP; ST_L_V 123, S0, V16, SP0
NOP; NOP; NOP; ST_L_V 123, S1, V17, SP1
NOP; NOP; NOP; ST_L_V 123, S2, V18, SP2
NOP; NOP; NOP; ST_L_V 123, S3, V19, SP3
NOP; NOP; NOP; ST_L_V 123, S4, V20, SP4
NOP; NOP; NOP; ST_L_V 123, S5, V21, SP5
NOP; NOP; NOP; ST_L_V 123, S6, V22, SP6
NOP; NOP; NOP; ST_L_V 123, S7, V23, SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S0, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S1, V17, SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S2, V18, SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S3, V19, SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S4, V20, SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S5, V21, SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S6, V22, SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S7, V23, SP7


NOP; NOP; NOP; ST_L_V 123, S0, V16, !SP0
NOP; NOP; NOP; ST_L_V 123, S1, V17, !SP1
NOP; NOP; NOP; ST_L_V 123, S2, V18, !SP2
NOP; NOP; NOP; ST_L_V 123, S3, V19, !SP3
NOP; NOP; NOP; ST_L_V 123, S4, V20, !SP4
NOP; NOP; NOP; ST_L_V 123, S5, V21, !SP5
NOP; NOP; NOP; ST_L_V 123, S6, V22, !SP6
NOP; NOP; NOP; ST_L_V 123, S7, V23, !SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S0, V16, !SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S1, V17, !SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S2, V18, !SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S3, V19, !SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S4, V20, !SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S5, V21, !SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S6, V22, !SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S7, V23, !SP7


NOP; NOP; NOP; ST_L_V 123, S0, VP6
NOP; NOP; NOP; ST_L_V 123, S1, VP7
NOP; NOP; NOP; ST_L_V 123, S2, VP8
NOP; NOP; NOP; ST_L_V 123, S3, VP9
NOP; NOP; NOP; ST_L_V 123, S4, VP0
NOP; NOP; NOP; ST_L_V 123, S5, VP1
NOP; NOP; NOP; ST_L_V 123, S6, VP2
NOP; NOP; NOP; ST_L_V 123, S7, VP3

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S0, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S1, VP7, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S2, VP8, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S3, VP9, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S4, VP0, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S5, VP1, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S6, VP2, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S7, VP3, SP0


NOP; NOP; NOP; ST_L_V 123, S0, VP6, SP0
NOP; NOP; NOP; ST_L_V 123, S1, VP7, SP1
NOP; NOP; NOP; ST_L_V 123, S2, VP8, SP2
NOP; NOP; NOP; ST_L_V 123, S3, VP9, SP3
NOP; NOP; NOP; ST_L_V 123, S4, VP0, SP4
NOP; NOP; NOP; ST_L_V 123, S5, VP1, SP5
NOP; NOP; NOP; ST_L_V 123, S6, VP2, SP6
NOP; NOP; NOP; ST_L_V 123, S7, VP3, SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S0, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S1, VP7, SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S2, VP8, SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S3, VP9, SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S4, VP0, SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S5, VP1, SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S6, VP2, SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S7, VP3, SP7


NOP; NOP; NOP; ST_L_V 123, S0, VP6, !SP0
NOP; NOP; NOP; ST_L_V 123, S1, VP7, !SP1
NOP; NOP; NOP; ST_L_V 123, S2, VP8, !SP2
NOP; NOP; NOP; ST_L_V 123, S3, VP9, !SP3
NOP; NOP; NOP; ST_L_V 123, S4, VP0, !SP4
NOP; NOP; NOP; ST_L_V 123, S5, VP1, !SP5
NOP; NOP; NOP; ST_L_V 123, S6, VP2, !SP6
NOP; NOP; NOP; ST_L_V 123, S7, VP3, !SP7

// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S0, VP6, !SP0
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S1, VP7, !SP1
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S2, VP8, !SP2
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S3, VP9, !SP3
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S4, VP0, !SP4
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S5, VP1, !SP5
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S6, VP2, !SP6
// CHECK: nop;    nop;    nop;    st_l_v 0x7b, S7, VP3, !SP7



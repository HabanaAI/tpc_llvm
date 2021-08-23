// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s

NOP; NOP; NOP; ST_L_V_LOW S0, V16
NOP; NOP; NOP; ST_L_V_LOW S1, V17
NOP; NOP; NOP; ST_L_V_LOW S2, V18
NOP; NOP; NOP; ST_L_V_LOW S3, V19
NOP; NOP; NOP; ST_L_V_LOW S4, V20
NOP; NOP; NOP; ST_L_V_LOW S5, V21
NOP; NOP; NOP; ST_L_V_LOW S6, V22
NOP; NOP; NOP; ST_L_V_LOW S7, V23

// CHECK: nop;    nop;    nop;    st_l_v_low S0, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S1, V17, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S2, V18, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S3, V19, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S4, V20, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S5, V21, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S6, V22, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S7, V23, SP0


NOP; NOP; NOP; ST_L_V_LOW S35, V16, SP0
NOP; NOP; NOP; ST_L_V_LOW S34, V17, SP1
NOP; NOP; NOP; ST_L_V_LOW S33, V18, SP2
NOP; NOP; NOP; ST_L_V_LOW S32, V19, SP3
NOP; NOP; NOP; ST_L_V_LOW S31, V20, SP4
NOP; NOP; NOP; ST_L_V_LOW S30, V21, SP5
NOP; NOP; NOP; ST_L_V_LOW S29, V22, SP6
NOP; NOP; NOP; ST_L_V_LOW S28, V23, SP7

// CHECK: nop;    nop;    nop;    st_l_v_low S35, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S34, V17, SP1
// CHECK: nop;    nop;    nop;    st_l_v_low S33, V18, SP2
// CHECK: nop;    nop;    nop;    st_l_v_low S32, V19, SP3
// CHECK: nop;    nop;    nop;    st_l_v_low S31, V20, SP4
// CHECK: nop;    nop;    nop;    st_l_v_low S30, V21, SP5
// CHECK: nop;    nop;    nop;    st_l_v_low S29, V22, SP6
// CHECK: nop;    nop;    nop;    st_l_v_low S28, V23, SP7


NOP; NOP; NOP; ST_L_V_LOW S0, V16, !SP0
NOP; NOP; NOP; ST_L_V_LOW S1, V17, !SP1
NOP; NOP; NOP; ST_L_V_LOW S2, V18, !SP2
NOP; NOP; NOP; ST_L_V_LOW S3, V19, !SP3
NOP; NOP; NOP; ST_L_V_LOW S4, V20, !SP4
NOP; NOP; NOP; ST_L_V_LOW S5, V21, !SP5
NOP; NOP; NOP; ST_L_V_LOW S6, V22, !SP6
NOP; NOP; NOP; ST_L_V_LOW S7, V23, !SP7

// CHECK: nop;    nop;    nop;    st_l_v_low S0, V16, !SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S1, V17, !SP1
// CHECK: nop;    nop;    nop;    st_l_v_low S2, V18, !SP2
// CHECK: nop;    nop;    nop;    st_l_v_low S3, V19, !SP3
// CHECK: nop;    nop;    nop;    st_l_v_low S4, V20, !SP4
// CHECK: nop;    nop;    nop;    st_l_v_low S5, V21, !SP5
// CHECK: nop;    nop;    nop;    st_l_v_low S6, V22, !SP6
// CHECK: nop;    nop;    nop;    st_l_v_low S7, V23, !SP7


NOP; NOP; NOP; ST_L_V_LOW S35, VP6
NOP; NOP; NOP; ST_L_V_LOW S34, VP7
NOP; NOP; NOP; ST_L_V_LOW S33, VP8
NOP; NOP; NOP; ST_L_V_LOW S32, VP9
NOP; NOP; NOP; ST_L_V_LOW S31, VP0
NOP; NOP; NOP; ST_L_V_LOW S30, VP1
NOP; NOP; NOP; ST_L_V_LOW S29, VP2
NOP; NOP; NOP; ST_L_V_LOW S28, VP3

// CHECK: nop;    nop;    nop;    st_l_v_low S35, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S34, VP7, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S33, VP8, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S32, VP9, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S31, VP0, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S30, VP1, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S29, VP2, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S28, VP3, SP0


NOP; NOP; NOP; ST_L_V_LOW S0, VP6, SP0
NOP; NOP; NOP; ST_L_V_LOW S1, VP7, SP1
NOP; NOP; NOP; ST_L_V_LOW S2, VP8, SP2
NOP; NOP; NOP; ST_L_V_LOW S3, VP9, SP3
NOP; NOP; NOP; ST_L_V_LOW S4, VP0, SP4
NOP; NOP; NOP; ST_L_V_LOW S5, VP1, SP5
NOP; NOP; NOP; ST_L_V_LOW S6, VP2, SP6
NOP; NOP; NOP; ST_L_V_LOW S7, VP3, SP7

// CHECK: nop;    nop;    nop;    st_l_v_low S0, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S1, VP7, SP1
// CHECK: nop;    nop;    nop;    st_l_v_low S2, VP8, SP2
// CHECK: nop;    nop;    nop;    st_l_v_low S3, VP9, SP3
// CHECK: nop;    nop;    nop;    st_l_v_low S4, VP0, SP4
// CHECK: nop;    nop;    nop;    st_l_v_low S5, VP1, SP5
// CHECK: nop;    nop;    nop;    st_l_v_low S6, VP2, SP6
// CHECK: nop;    nop;    nop;    st_l_v_low S7, VP3, SP7


NOP; NOP; NOP; ST_L_V_LOW S35, VP6, !SP0
NOP; NOP; NOP; ST_L_V_LOW S34, VP7, !SP1
NOP; NOP; NOP; ST_L_V_LOW S33, VP8, !SP2
NOP; NOP; NOP; ST_L_V_LOW S32, VP9, !SP3
NOP; NOP; NOP; ST_L_V_LOW S31, VP0, !SP4
NOP; NOP; NOP; ST_L_V_LOW S30, VP1, !SP5
NOP; NOP; NOP; ST_L_V_LOW S29, VP2, !SP6
NOP; NOP; NOP; ST_L_V_LOW S28, VP3, !SP7

// CHECK: nop;    nop;    nop;    st_l_v_low S35, VP6, !SP0
// CHECK: nop;    nop;    nop;    st_l_v_low S34, VP7, !SP1
// CHECK: nop;    nop;    nop;    st_l_v_low S33, VP8, !SP2
// CHECK: nop;    nop;    nop;    st_l_v_low S32, VP9, !SP3
// CHECK: nop;    nop;    nop;    st_l_v_low S31, VP0, !SP4
// CHECK: nop;    nop;    nop;    st_l_v_low S30, VP1, !SP5
// CHECK: nop;    nop;    nop;    st_l_v_low S29, VP2, !SP6
// CHECK: nop;    nop;    nop;    st_l_v_low S28, VP3, !SP7


NOP; NOP; NOP; ST_L_V_LOW 123, V16
NOP; NOP; NOP; ST_L_V_LOW 123, V17
NOP; NOP; NOP; ST_L_V_LOW 123, V18
NOP; NOP; NOP; ST_L_V_LOW 123, V19
NOP; NOP; NOP; ST_L_V_LOW 123, V20
NOP; NOP; NOP; ST_L_V_LOW 123, V21
NOP; NOP; NOP; ST_L_V_LOW 123, V22
NOP; NOP; NOP; ST_L_V_LOW 123, V23

// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V17, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V18, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V19, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V20, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V21, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V22, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V23, SP0


NOP; NOP; NOP; ST_L_V_LOW 123, V16, SP0
NOP; NOP; NOP; ST_L_V_LOW 123, V17, SP1
NOP; NOP; NOP; ST_L_V_LOW 123, V18, SP2
NOP; NOP; NOP; ST_L_V_LOW 123, V19, SP3
NOP; NOP; NOP; ST_L_V_LOW 123, V20, SP4
NOP; NOP; NOP; ST_L_V_LOW 123, V21, SP5
NOP; NOP; NOP; ST_L_V_LOW 123, V22, SP6
NOP; NOP; NOP; ST_L_V_LOW 123, V23, SP7

// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V16, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V17, SP1
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V18, SP2
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V19, SP3
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V20, SP4
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V21, SP5
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V22, SP6
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V23, SP7


NOP; NOP; NOP; ST_L_V_LOW 123, V16, !SP0
NOP; NOP; NOP; ST_L_V_LOW 123, V17, !SP1
NOP; NOP; NOP; ST_L_V_LOW 123, V18, !SP2
NOP; NOP; NOP; ST_L_V_LOW 123, V19, !SP3
NOP; NOP; NOP; ST_L_V_LOW 123, V20, !SP4
NOP; NOP; NOP; ST_L_V_LOW 123, V21, !SP5
NOP; NOP; NOP; ST_L_V_LOW 123, V22, !SP6
NOP; NOP; NOP; ST_L_V_LOW 123, V23, !SP7

// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V16, !SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V17, !SP1
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V18, !SP2
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V19, !SP3
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V20, !SP4
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V21, !SP5
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V22, !SP6
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, V23, !SP7


NOP; NOP; NOP; ST_L_V_LOW 123, VP6
NOP; NOP; NOP; ST_L_V_LOW 123, VP7
NOP; NOP; NOP; ST_L_V_LOW 123, VP8
NOP; NOP; NOP; ST_L_V_LOW 123, VP9
NOP; NOP; NOP; ST_L_V_LOW 123, VP0
NOP; NOP; NOP; ST_L_V_LOW 123, VP1
NOP; NOP; NOP; ST_L_V_LOW 123, VP2
NOP; NOP; NOP; ST_L_V_LOW 123, VP3

// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP7, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP8, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP9, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP0, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP1, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP2, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP3, SP0


NOP; NOP; NOP; ST_L_V_LOW 123, VP6, SP0
NOP; NOP; NOP; ST_L_V_LOW 123, VP7, SP1
NOP; NOP; NOP; ST_L_V_LOW 123, VP8, SP2
NOP; NOP; NOP; ST_L_V_LOW 123, VP9, SP3
NOP; NOP; NOP; ST_L_V_LOW 123, VP0, SP4
NOP; NOP; NOP; ST_L_V_LOW 123, VP1, SP5
NOP; NOP; NOP; ST_L_V_LOW 123, VP2, SP6
NOP; NOP; NOP; ST_L_V_LOW 123, VP3, SP7

// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP6, SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP7, SP1
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP8, SP2
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP9, SP3
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP0, SP4
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP1, SP5
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP2, SP6
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP3, SP7


NOP; NOP; NOP; ST_L_V_LOW 123, VP6, !SP0
NOP; NOP; NOP; ST_L_V_LOW 123, VP7, !SP1
NOP; NOP; NOP; ST_L_V_LOW 123, VP8, !SP2
NOP; NOP; NOP; ST_L_V_LOW 123, VP9, !SP3
NOP; NOP; NOP; ST_L_V_LOW 123, VP0, !SP4
NOP; NOP; NOP; ST_L_V_LOW 123, VP1, !SP5
NOP; NOP; NOP; ST_L_V_LOW 123, VP2, !SP6
NOP; NOP; NOP; ST_L_V_LOW 123, VP3, !SP7

// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP6, !SP0
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP7, !SP1
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP8, !SP2
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP9, !SP3
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP0, !SP4
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP1, !SP5
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP2, !SP6
// CHECK: nop;    nop;    nop;    st_l_v_low 0x7b, VP3, !SP7

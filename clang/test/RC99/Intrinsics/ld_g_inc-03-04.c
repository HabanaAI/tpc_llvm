// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s
// RUN: %codegen -emit-obj -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o %t.g3.o
// RUN: %disasm --mcpu=goya2 %t.g3.o | FileCheck --check-prefixes=CHECK-ASM %s

// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -emit-obj -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o %t.o
// RUN: %disasm --mcpu=gaudi2 %t.o | FileCheck --check-prefixes=CHECK-ASM %s

// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s
// RUN: %codegen -emit-obj -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu=doron1 %t.o | FileCheck --check-prefixes=CHECK-ASM %s

void main(unsigned dest, tensor out, int pred) {
int5 indx = { 0, 0, 0, 0, 0 };
//void __global *addr = gen_addr(indx, out, 0, 0, 1, 0);

{
	char inc0 = 0;
 	char __local *dptr = (char __local *)dest++;
        char __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
	char res0 = s_i8_ld_g_inc(&addr, 0, inc0, 1, 0);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g inc_1 %S{{[0-9]+}}, %AD{{[0-9]+}}
    // CHECK-ASM-DAG: ld_g inc_1 S{{[0-9]+}}, AD{{[0-9]+}}
}

{
	half inc0 = 0;
 	half __local *dptr = (half __local *)dest++;
        half __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
	half res0 = s_f16_ld_g_inc(&addr, SW_INC_8, inc0, 1, 0);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g inc_8 %S{{[0-9]+}}, %AD{{[0-9]+}}
    // CHECK-ASM-DAG: ld_g inc_8 S{{[0-9]+}}, AD{{[0-9]+}}

}

{
	bf16 inc0 = 0;
 	bf16 __local *dptr = (bf16 __local *)dest++;
        bf16 __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
	bf16 res0 = s_bf16_ld_g_inc(&addr, SW_INC_2|SW_L0CS, inc0, pred, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g inc_2 l0cs %S{{[0-9]+}}, %AD{{[0-9]+}}, !%SP{{[0-9]+}}
    // CHECK-ASM-DAG: ld_g inc_2 l0cs S{{[0-9]+}}, AD{{[0-9]+}}, !SP{{[0-9]+}}
}

{
	float inc0 = 0;
 	float __local *dptr = (float __local *)dest++;
        float __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
	float res0 = s_f32_ld_g_inc(&addr, 0, inc0, pred, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g inc_4 %S{{[0-9]+}}, %AD{{[0-9]+}}, !%SP{{[0-9]+}}
    // CHECK-ASM-DAG: ld_g inc_4 S{{[0-9]+}}, AD{{[0-9]+}}, !SP{{[0-9]+}}
}

{
	short inc0 = 0;
 	short __local *dptr = (short __local *)dest++;
        short __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
	short res0 = s_i16_ld_g_inc(&addr, 0, inc0, pred, 0);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g inc_2 %S{{[0-9]+}}, %AD{{[0-9]+}}, %SP{{[0-9]+}}
    // CHECK-ASM-DAG: ld_g inc_2 S{{[0-9]+}}, AD{{[0-9]+}}, SP{{[0-9]+}}
}

{
	unsigned short inc0 = 0;
 	unsigned short __local *dptr = (unsigned short __local *)dest++;
        unsigned short __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
	unsigned short res0 = s_u16_ld_g_inc(&addr, SW_INC_4 | SW_EV_HINT, inc0, pred, 0);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g inc_4 ev_hint %S{{[0-9]+}}, %AD{{[0-9]+}}, %SP{{[0-9]+}}
    // CHECK-ASM-DAG: ld_g inc_4 ev_hint S{{[0-9]+}}, AD{{[0-9]+}}, SP{{[0-9]+}}
}

{
	_Bool inc0 = 0;
 	_Bool __local *dptr = (_Bool __local *)dest++;
        _Bool __global *addr = gen_addr(indx, out, 0, 0, 1, 0);
	_Bool res0 = s_i1_ld_g_inc(&addr, 0, inc0, pred, 1);
  	*dptr++ = res0;
    // CHECK-DAG: ld_g inc_1 %SP{{[0-9]+}}, %AD{{[0-9]+}}, !%SP{{[0-9]+}}
    // CHECK-ASM-DAG: ld_g inc_1 SP{{[0-9]+}}, AD{{[0-9]+}}, !SP{{[0-9]+}}
}
}

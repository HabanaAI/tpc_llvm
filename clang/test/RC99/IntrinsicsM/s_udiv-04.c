// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

// XFAIL: *
// GAUDI-2176
void main(int x0, int x1, _Bool pred, int dest)
{
    void __local *dest_void_ptr = (void  __local *)dest;

    unsigned __local *src1_uint32_ptr = (unsigned __local *)x0;
    unsigned __local *src2_uint32_ptr = (unsigned __local *)x1;
    unsigned short __local *src1_uint16_ptr = (unsigned short __local *)x0;
    unsigned short __local *src2_uint16_ptr = (unsigned short __local *)x1;
    unsigned char __local *src1_uint8_ptr = (unsigned char __local *)x0;
    unsigned char __local *src2_uint8_ptr = (unsigned char __local *)x1;

    unsigned src1_uint32 = *src1_uint32_ptr;
    unsigned src2_uint32 = *src2_uint32_ptr;
    unsigned short src1_uint16 = *src1_uint16_ptr;
    unsigned short src2_uint16 = *src2_uint16_ptr;
    unsigned char src1_uint8 = *src1_uint8_ptr;
    unsigned char src2_uint8 = *src2_uint8_ptr;

    {
        unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
        unsigned res = 0;
        res = u32_udiv(src1_uint32, src2_uint32, SW_DIV_MODE_DIV, res, pred, 0);
        *dest_ptr++ = res;
        // CHECK-DAG: udiv.u32 div {{%S[0-9]+}}, {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
        dest_void_ptr = dest_ptr;
    }
    {
        unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
        unsigned res = 0;
        res = u32_udiv(src1_uint32, src2_uint32, SW_DIV_MODE_MOD, res, pred, 0);
        *dest_ptr++ = res;
        // CHECK-DAG: udiv.u32 mod {{%S[0-9]+}}, {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
        dest_void_ptr = dest_ptr;
    }
    
    {
        unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
        unsigned short res = 0;
        res = u16_udiv(src1_uint16, src2_uint16, SW_DIV_MODE_DIV, res, pred, 0);
        *dest_ptr++ = res;
        // CHECK-DAG: udiv.u16 div {{%S[0-9]+}}, {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
        dest_void_ptr = dest_ptr;
    }
    {
        unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
        unsigned char res = 0;
        res = u8_udiv(src1_uint8, src2_uint8, SW_DIV_MODE_DIV, res, pred, 0);
        *dest_ptr++ = res;
        // CHECK-DAG: udiv.u8 div {{%S[0-9]+}}, {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
        dest_void_ptr = dest_ptr;
    }

    {
        unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
        uint32_t_pair_t res = {0, 0};
        res = u32_udiv_both(src1_uint32, src2_uint32, 0, res, pred, 0);
        *dest_ptr++ = res.v1;
        *dest_ptr++ = res.v2;
        // CHECK-DAG: udiv.u32 both_div_mod {{%Z[0-9]+}}, {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
        dest_void_ptr = dest_ptr;
    }
    {
        unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
        uint16_t_pair_t res = {0, 0};
        res = u16_udiv_both(src1_uint16, src2_uint16, 0, res, pred, 0);
        *dest_ptr++ = res.v1;
        *dest_ptr++ = res.v2;
        // CHECK-DAG: udiv.u16 both_div_mod {{%Z[0-9]+}}, {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
        dest_void_ptr = dest_ptr;
    }
    {
        unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
        uint8_t_pair_t res = {0, 0};
        res = u8_udiv_both(src1_uint8, src2_uint8, 0, res, pred, 0);
        *dest_ptr++ = res.v1;
        *dest_ptr++ = res.v2;
        // CHECK-DAG: udiv.u8 both_div_mod {{%Z[0-9]+}}, {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
        dest_void_ptr = dest_ptr;
    }
}

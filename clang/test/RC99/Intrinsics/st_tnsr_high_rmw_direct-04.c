// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned addr1, unsigned addr2, int dest, _Bool pred, int rmw) {
  int64 __local *vector_ptr = (int64 __local *)dest;

  {
    float64 __local *dest_ptr = (float64 __local *)vector_ptr;
    float64 res = 0;
    
    v_f32_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    v_i32_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    uint64 __local *dest_ptr = (uint64 __local *)vector_ptr;
    uint64 res = 0;

    v_u32_st_tnsr_high_rmw_direct(addr1 + 1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    short128 __local *dest_ptr = (short128 __local *)vector_ptr;
    short128 res = 0;

    v_i16_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    ushort128 __local *dest_ptr = (ushort128 __local *)vector_ptr;
    ushort128 res = 0;

    v_u16_st_tnsr_high_rmw_direct(addr1 + 1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    char256 __local *dest_ptr = (char256 __local *)vector_ptr;
    char256 res = 0;

    v_i8_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    uchar256 __local *dest_ptr  = (uchar256 __local *)vector_ptr;
    uchar256 res  = 0;

    v_u8_st_tnsr_high_rmw_direct(addr1 + 1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    half128 __local *dest_ptr = (half128 __local *)vector_ptr;
    half128 res = 0;
    
    v_f16_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    v_i32_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, SW_PCK_32_TO_8, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel PCK_32_TO_8 direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    v_i32_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, SW_PACK, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel pack direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }

  {
    minihalf256  __local *dest_ptr = (minihalf256 __local *)vector_ptr;
    minihalf256 res = 0;

    v_h8_st_tnsr_high_rmw_direct(addr1, addr2, res, rmw, SW_PACK, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (minihalf256 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high rmw_sel pack direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }
}

// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(int input, int dest, _Bool pred) {
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)vector_ptr;
    minifloat256 res = 0;
    
    v_f8_st_tnsr(index, input, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr %ST_TNSR_ID_REG, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    v_f8_st_tnsr(index, 0, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    v_f8_st_tnsr_high(index, input, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high %ST_TNSR_ID_REG, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    v_f8_st_tnsr_high(index, 0, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    v_f8_st_tnsr_low(index, input, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_low %ST_TNSR_ID_REG, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    v_f8_st_tnsr_low(index, 0, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_low 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }

  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)vector_ptr;
    minihalf256 res = 0;
    
    v_h8_st_tnsr(index, input, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr %ST_TNSR_ID_REG, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    v_h8_st_tnsr(index, 0, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    v_h8_st_tnsr_high(index, input, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high %ST_TNSR_ID_REG, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    v_h8_st_tnsr_high(index, 0, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_high 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    v_h8_st_tnsr_low(index, input, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_low %ST_TNSR_ID_REG, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    v_h8_st_tnsr_low(index, 0, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_low 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
}

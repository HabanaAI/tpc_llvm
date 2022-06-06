// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s

void main(tensor input, int other_input, _Bool pred) {
  int5 index = {0};
  
  squeeze_cntr sqz = 0;
  // CHECK-DAG: mov  %SQZ_CNTR[[NUM:[0-7]]]_HI, %S{{[0-9]+}}
  // CHECK-DAG: mov  %SQZ_CNTR[[NUM]]_LO, %S{{[0-9]+}}

  {
    st_tnsr_s_hwr(index, input, sqz, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg 0x0, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %SP{{[0-9]+}}
    st_tnsr_s_hwr(index, other_input, sqz, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg %ST_TNSR_ID_REG, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %SP{{[0-9]+}}
    st_tnsr_s_hwr_rmw(index, input, sqz, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg rmw_sel 0x0, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %ST_RMW_REG, %SP{{[0-9]+}}
    st_tnsr_s_hwr_rmw(index, other_input, sqz, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %ST_RMW_REG, %SP{{[0-9]+}}
  }
  
  {
    st_tnsr_s_hwr(index, input, sqz, SW_ST_TNSR_S_BV64, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg bv64 0x0, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %SP{{[0-9]+}}
    st_tnsr_s_hwr(index, other_input, sqz, SW_ST_TNSR_S_BV64, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg bv64 %ST_TNSR_ID_REG, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %SP{{[0-9]+}}
    st_tnsr_s_hwr_rmw(index, input, sqz, 1, SW_ST_TNSR_S_BV64, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg rmw_sel bv64 0x0, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %ST_RMW_REG, %SP{{[0-9]+}}
    st_tnsr_s_hwr_rmw(index, other_input, sqz, 1, SW_ST_TNSR_S_BV64, pred, 0);
    // CHECK-DAG: st_tnsr_s hw_reg rmw_sel bv64 %ST_TNSR_ID_REG, %I{{[0-9]+}}, %SQZ_CNTR[[NUM]], %ST_RMW_REG, %SP{{[0-9]+}}
  }
}

// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

typedef struct _ushort128_bfloat128_pair_t  ushort128_bfloat128;
typedef struct _ushort128_half128_pair_t  ushort128_half128;

void main(int x0a, int desta, _Bool pred)
{
  int128 __local *dest_ptr = (int128 __local *)desta;

// bf16
  bfloat128 __local *x0_bf16_ptr = (bfloat128 __local *)x0a;  
  ushort128_bfloat128 __local *dest_bf16_ptr = (ushort128_bfloat128  __local *)dest_ptr;
  bfloat128 x0_bf16 = *x0_bf16_ptr;
  ushort128_bfloat128 res_bf16 = *dest_bf16_ptr;
  
  res_bf16 = v_bf16_get_lut_entry_and_interval_start_b(x0_bf16, 0, SW_LUT_OPT, res_bf16, pred, 0);
  *dest_bf16_ptr++ = res_bf16;

  res_bf16 = v_bf16_get_lut_entry_and_interval_start_b(x0_bf16, 0, SW_LUT_EXP0, res_bf16, pred, 0);
  *dest_bf16_ptr++ = res_bf16;

  res_bf16 = v_bf16_get_lut_entry_and_interval_start_b(x0_bf16, 0, SW_LUT_OPT | SW_LUT_EXP0, res_bf16, pred, 0);
  *dest_bf16_ptr++ = res_bf16;

  dest_ptr = (int128 __local *)dest_bf16_ptr;

// CHECK-DAG: get_lut_entry_and_interval_start.bf16 opt %D{{[0-9]+}}, %V{{[0-9]+}}, 0x0, %SP{{[0-9]+}}
// CHECK-DAG: get_lut_entry_and_interval_start.bf16 exp0 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x0, %SP{{[0-9]+}}
// CHECK-DAG: get_lut_entry_and_interval_start.bf16 opt exp0 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x0, %SP{{[0-9]+}}
  
// f16
  half128 __local *x0_f16_ptr = (half128 __local *)x0a;
  ushort128_half128 __local *dest_f16_ptr = (ushort128_half128 __local *)dest_ptr;
  half128 x0_f16 = *x0_f16_ptr;
  ushort128_half128 res_f16 = *dest_f16_ptr;


  res_f16 = v_f16_get_lut_entry_and_interval_start_b(x0_f16, 0, SW_LUT_EXP0, res_f16, pred, 0);
  *dest_f16_ptr++ = res_f16;

// CHECK-DAG: get_lut_entry_and_interval_start.f16 exp0 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x0, %SP{{[0-9]+}}
}

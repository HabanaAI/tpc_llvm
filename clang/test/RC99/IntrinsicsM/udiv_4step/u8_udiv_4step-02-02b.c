// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudib %s -o - | FileCheck %s

void main(unsigned char x0, unsigned char x1, int dest, _Bool pred)
{
  const unsigned step_1 = 4;
  unsigned char __local *res0 = (unsigned char __local *)dest;
  uint8_t_pair_t temp_res0 = {0,0};
  uint8_t_pair_t temp_res1 = {0,0};
  temp_res0 = u8_udiv_4step(x0, 1, 0, temp_res0, pred, 0);
  //CHECK-DAG: udiv_4step.u8  0x1 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  *res0++ = temp_res0.v1;
  temp_res1 = u8_udiv_4step(x1, step_1, 0, temp_res1, pred, 0);
  //CHECK-DAG: udiv_4step.u8  0x4 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  *res0++ = temp_res1.v1;
}

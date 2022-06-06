// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

void main(tensor ifm, tensor ofm)
{
  const int5 index_space_start = get_index_space_offset();
  int5 Ind;
  char256 Val;
  Val = 0; //v_i8_ld_tnsr_b(Ind, ifm, 0, 1, 0, );

  const int row0 = index_space_start[2];
  const int row1 = row0 + 1;
  const int row2 = row0 + 2;
  Ind[0] = row1;
  Ind[1] = row1;
  Ind[3] = row2;
  Ind[2] = row0;
  Ind[4] = row0;
  v_i8_st_tnsr(Ind, ofm, Val, 0, 1, 0);
}

// CHECK: 	set_indx [[IRF:%I[0-9]+]], b00011, %S{{[0-9]+}}
// CHECK:	set_indx [[IRF]], b10100, %S{{[0-9]+}}
// CHECK:	set_indx [[IRF]], b01000, %S{{[0-9]+}}
// CHECK:	st_tnsr  0x1, [[IRF]], %V{{[0-9]+}}

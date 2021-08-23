// RUN: %clang_cc1 -triple -tpc-none-none -std=rc99 -S -O1 -target-cpu dali %s -o - | FileCheck --check-prefix=CHECK-DALI %s
// RUN: %clang_cc1 -triple -tpc-none-none -std=rc99 -S -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s


void main(int dest) {
  char256 __local *dptr = (char256 __local *) dest;
  write_lfsr(*dptr);
  *dptr = read_lfsrnc();
// CHECK-DALI:       WRITE   LFSR, %V{{[0-9]+}}
// CHECK-DALI-NEXT:  nop
// CHECK-DALI-NEXT:  nop
// CHECK-DALI-NEXT:  nop
// CHECK-DALI-NEXT:  READ    %V{{[0-9]+}}, LFSR_NO_CHANGE

// CHECK-GAUDI:      WRITE   LFSR, %V{{[0-9]+}}
// CHECK-GAUDI-NEXT: READ    %V{{[0-9]+}}, LFSR_NO_CHANGE
}

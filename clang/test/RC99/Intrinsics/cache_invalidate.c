// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P,GEN4P %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P,GEN4P %s

void main(_Bool x) {
  cache_invalidate(0, 1, 0);
// CHECK: cache_invalidate

  cache_invalidate(0, x, 0);
// CHECK: cache_invalidate %SP{{[0-9]+}}

#if defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  cache_invalidate(SW_SB, 1, 0);
// GEN3P: cache_invalidate sb

  cache_invalidate(SW_SB, x, 0);
// GEN3P: cache_invalidate sb %SP{{[0-9]+}}
#endif

#if defined(__gaudi2__) || defined(__doron1__)
  cache_invalidate(SW_D, 1, 0);
// GEN4P: cache_invalidate d
  cache_invalidate(SW_D, x, 0);
// GEN4P: cache_invalidate d %SP{{[0-9]+}}

  cache_invalidate(SW_LU, 1, 0);
// GEN4P: cache_invalidate lu
  cache_invalidate(SW_LU, x, 0);
// GEN4P: cache_invalidate lu %SP{{[0-9]+}}

  cache_invalidate(SW_RST_LU, 1, 0);
// GEN4P: cache_invalidate rst_lu
  cache_invalidate(SW_RST_LU, x, 0);
// GEN4P: cache_invalidate rst_lu %SP{{[0-9]+}}

  cache_invalidate(SW_RST_D_PREF, 1, 0);
// GEN4P: cache_invalidate rst_d_pref
  cache_invalidate(SW_RST_D_PREF, x, 0);
// GEN4P: cache_invalidate rst_d_pref %SP{{[0-9]+}}
#endif
}


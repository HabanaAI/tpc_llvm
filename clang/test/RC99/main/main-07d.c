// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -max-tensors 16 -verify %s
// GAUDI-2311
// XFAIL: *
#pragma tpc_printf (enable)
void main(tensor T0,
          tensor T1,
          tensor T2,
          tensor T3,
          tensor T4,
          tensor T5,
          tensor T6,
          tensor T7,
          tensor T8,
          tensor T9,
          tensor T10,
          tensor T11,
          tensor T12,
          tensor T13,
//          tensor T14,
//          tensor T15,
          tensor T16,  // expected-error{{too many tensors are declared}}
          int arg0, int arg1, int arg2, int arg3, int arg4,
  int arg5, int arg6, int arg7, int arg8, int arg9,
  int arg10, int arg11, int arg12, int arg13, int arg14,
  int arg15, int arg16, int arg17, int arg18, int arg19,
  int arg20, int arg21, int arg22, int arg23, int arg24,
  int arg25, int arg26, int arg27, int arg28, int arg29,
  int arg30, float float34, int arg31) {
}

// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

// By default the architecture is dali, which allows 8 tensors.
void main(tensor T0,
          tensor T1,
          tensor T2,
          tensor T3,
          tensor T4,
          tensor T5,
          tensor T6,
          tensor T7,
          tensor T8,  // expected-error{{too many tensors are declared}}
          tensor T9,
          tensor T10,
          tensor T11,
          tensor T12,
          tensor T13,
          tensor T14,
          tensor T15,
          tensor T16,
          float f) {
}

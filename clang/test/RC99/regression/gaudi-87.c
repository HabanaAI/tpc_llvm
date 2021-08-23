// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

void main(int dest, int x) {
  float __local *s_ptr = (float __local *) dest;
  float64 __local *v_ptr = (float64 __local *)x;
  uint64 v_uint = 0;
  float aaa = (float)v_uint; // expected-error{{invalid conversion between vector type 'uint64' (vector of 64 'unsigned int' values) and scalar type 'float'}}
  float64 v1_float = *((float64*)&v_uint); // legal
  float v2_float = *((float *)&v_uint); // expected-error{{invalid conversion between vector type 'uint64' (vector of 64 'unsigned int' values) and scalar type '__attribute__((address_space(1))) float'}}
  float64 v3_float = *((float64 *)&aaa); // expected-error{{invalid conversion between scalar type 'float' and vector type '__attribute__((address_space(2))) float64' (vector of 64 'float' values)}}
  float v4_float = *((float *)v_ptr); // expected-error{{invalid conversion between vector type '__attribute__((address_space(2))) float64' (vector of 64 'float' values) and scalar type '__attribute__((address_space(1))) float'}}
  float64 v5_float = *((float64 *)v_ptr); // legal
  float64 v6_float = *((float64 *)s_ptr); // expected-error{{invalid conversion between scalar type '__attribute__((address_space(1))) float' and vector type '__attribute__((address_space(2))) float64' (vector of 64 'float' values)}}
  float v7_float = *((float *)s_ptr);  // legal
}

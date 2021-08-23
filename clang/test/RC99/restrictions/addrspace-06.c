// RUN: %clang_cc1 -fsyntax-only -std=rc99 -std=rc99 -triple tpc-none-none -verify %s

struct New {
  __global__ float * global_var_ptr;  // expected-error{{fields cannot be global pointers}}
  __local__ float * local_var_ptr;
  __local__ float64 * local_vec_ptr;
  __global__ float64 * global_vec_ptr;
};

__local__  struct New local_struc; 
__global__  struct New global_struc; // expected-error{{variables in global address space are not allowed}}

__global__ int * __local locvar_globptr[4]; // expected-error{{arrays of global pointers are not allowed}}
__local__ int * __global globvar_locptr[4]; // expected-error{{variables in global address space are not allowed}}
__global__ int * __global globvar_globptr[4]; // expected-error{{variables in global address space are not allowed}}
__global__ int64 * __local locvar_globvecptr[4]; // expected-error{{arrays of global pointers are not allowed}}
__local__ int64 * __global globvar_locvecptr[4]; // expected-error{{variables in global address space are not allowed}}
__global__ int64 * __global globvar_globvecptr[4]; // expected-error{{variables in global address space are not allowed}}

void main() {}


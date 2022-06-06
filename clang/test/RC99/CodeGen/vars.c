// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -disable-llvm-passes %s -o - | FileCheck %s

char var_char = 'a';
short var_short = 12;
int var_int = 44;
float var_float = 123.456;

// CHECK: @var_char = dso_local addrspace(1)
// CHECK: @var_short = dso_local addrspace(1)
// CHECK: @var_int = dso_local addrspace(1)
// CHECK: @var_float = dso_local addrspace(1)

bool256 var_bool256 = 0;
char256 var_char256 = 1;
uchar256 var_uchar256 = 3;
short128 var_short128 = -1;
ushort128 var_ushort128 = 5;
int64 var_int64 = 66;
uint64 var_uint64 = 77;
float64 var_float64 = 0.5;

// CHECK: @var_bool256 = dso_local addrspace(2)
// CHECK: @var_char256 = dso_local addrspace(2)
// CHECK: @var_uchar256 = dso_local addrspace(2)
// CHECK: @var_short128 = dso_local addrspace(2)
// CHECK: @var_ushort128 = dso_local addrspace(2)
// CHECK: @var_int64 = dso_local addrspace(2)
// CHECK: @var_uint64 = dso_local addrspace(2)
// CHECK: @var_float64 = dso_local addrspace(2)

int256 var_int256 = { 0 };
uint256 var_uint256 = { 10 };
// CHECK: @var_int256 = dso_local addrspace(2)
// CHECK: @var_uint256 = dso_local addrspace(2)

uint8_t_pair_t var_uint8_t_pair_t = { 1, 1 };
// CHECK: @var_uint8_t_pair_t = dso_local addrspace(1)

uchar256_pair_t var_uchar256_pair_t = { 0, 0 };
// CHECK: @var_uchar256_pair_t = dso_local addrspace(2)

char *var_char_ptr = 0;
char __local *var_char_lptr = 0;
// CHECK: @var_char_ptr = dso_local addrspace(1)
// CHECK: @var_char_lptr = dso_local addrspace(1)

char *var_char256_ptr = 0;
char __local *var_char256_lptr = 0;
// CHECK: @var_char256_ptr = dso_local addrspace(1)
// CHECK: @var_char256_lptr = dso_local addrspace(1)



char var_char_a[12];
// CHECK: @var_char_a = dso_local addrspace(1)

char256 var_char256_a[4];
// CHECK: @var_char256_a = dso_local addrspace(2)



void main() {
}

// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -disable-llvm-passes %s -o - | FileCheck %s

char var_char = 'a';
short var_short = 12;
int var_int = 44;
float var_float = 123.456;

// CHECK: @var_char = addrspace(1)
// CHECK: @var_short = addrspace(1)
// CHECK: @var_int = addrspace(1)
// CHECK: @var_float = addrspace(1)

bool256 var_bool256 = 0;
char256 var_char256 = 1;
uchar256 var_uchar256 = 3;
short128 var_short128 = -1;
ushort128 var_ushort128 = 5;
int64 var_int64 = 66;
uint64 var_uint64 = 77;
float64 var_float64 = 0.5;

// CHECK: @var_bool256 = addrspace(2)
// CHECK: @var_char256 = addrspace(2)
// CHECK: @var_uchar256 = addrspace(2)
// CHECK: @var_short128 = addrspace(2)
// CHECK: @var_ushort128 = addrspace(2)
// CHECK: @var_int64 = addrspace(2)
// CHECK: @var_uint64 = addrspace(2)
// CHECK: @var_float64 = addrspace(2)

int256 var_int256 = { 0 };
uint256 var_uint256 = { 10 };
// CHECK: @var_int256 = addrspace(2)
// CHECK: @var_uint256 = addrspace(2)

uint8_t_pair_t var_uint8_t_pair_t = { 1, 1 };
// CHECK: @var_uint8_t_pair_t = addrspace(1)

uchar256_pair_t var_uchar256_pair_t = { 0, 0 };
// CHECK: @var_uchar256_pair_t = addrspace(2)

char *var_char_ptr = 0;
char __local *var_char_lptr = 0;
// CHECK: @var_char_ptr = addrspace(1)
// CHECK: @var_char_lptr = addrspace(1)

char *var_char256_ptr = 0;
char __local *var_char256_lptr = 0;
// CHECK: @var_char256_ptr = addrspace(1)
// CHECK: @var_char256_lptr = addrspace(1)



char var_char_a[12];
// CHECK: @var_char_a = {{.*}} addrspace(1)

char256 var_char256_a[4];
// CHECK: @var_char256_a = {{.*}} addrspace(2)



void main() {
}

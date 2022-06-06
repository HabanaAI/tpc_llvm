// RUN: %clang_cc1 -emit-llvm-bc -std=rc99 -triple tpc -tpc-library -DLIB %s -o %t.bc
// RUN: %clang_cc1 -S -emit-llvm -std=rc99 -triple tpc -mlink-bitcode-file %t.bc %s -o - | FileCheck %s

#ifdef LIB
int qqq(int x) {
  return x + 22;
}

#else

int qqq(int x);

void main(int dest, int src) {
  int *dptr = (int *)dest;
  *dptr = qqq(src);
}
#endif

// CHECK: define dso_local void @main(i32 %dest, i32 %src) {{.*}}#0 {
// CHECK: entry:
// CHECK:   %0 = inttoptr i32 %dest to i32 addrspace(1)*
// CHECK:   %add.i = add nsw i32 %src, 22
// CHECK:   store i32 %add.i, i32 addrspace(1)* %0, align 4
// CHECK:   ret void
// CHECK: }
// CHECK: attributes #0 = { {{.*}} "tpc-kernel"="true" {{.*}} }

; RUN: not opt -passes=link-tpc-headers -ILTO=%t.missing.so -S %s 2>&1 | FileCheck %s

; A library that cannot be loaded is a usage error; it used to be an
; assertion failure.

; CHECK: LLVM ERROR: -ILTO: {{.*}}missing.so: cannot open shared object file
; CHECK-NOT: Assertion

target triple = "tpc"

define void @main() {
  ret void
}

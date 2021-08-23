// RUN: not %clang -target tpc -std=rc99 %s -o - 2>&1 | FileCheck --check-prefix=NOLINK %s
// NOLINK: error: linker is unavailable for this target

// RUN: not %clang -target tpc -std=rc99 %s %s.xxx -o - 2>&1 | FileCheck --check-prefix=MULTI %s
// MULTI: error: only one input file is allowed in compilation for this target



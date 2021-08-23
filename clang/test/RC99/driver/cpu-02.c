// RUN: %clang -target tpc -std=rc99 -c -march=dali %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -march=goya %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -march=gaudi %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI %s





// RUN: %clang -target tpc -std=rc99 -c -mtune=dali %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=goya %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=gaudi %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI %s





// RUN: %clang -target tpc -std=rc99 -c -mcpu=dali %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=goya %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=gaudi %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI %s





void main() {
}

// GOYA: "-target-cpu" "goya"
// GAUDI: "-target-cpu" "gaudi"

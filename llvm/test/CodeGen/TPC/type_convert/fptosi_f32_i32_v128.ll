; RUN: llc -march=tpc -mcpu=goya2 -O0 %s -o - | FileCheck %s

; CHECK:  ld_l_v  %V{{[0-9]+}}, 0x100
; CHECK:	ld_l_v  %V[[M:[0-9]+]], 0x0
;
; CHECK: convert.f32 all_lanes target_type=int32  %D[[N:[0-9]+]], %D[[M:[0-9]+]]
;
; CHECK: st_l_v  0x0, %V[[N]]
; CHECK: st_l_v  0x100, %V{{[0-9]+}}
; CHECK: halt
;
; TODO: validate [[#N+1]] and [[#M+1]] with a later FileCheck version

target triple = "tpc"

%struct._int64_pair_t = type { <64 x i32>, <64 x i32> }
%struct._float64_pair_t = type { <64 x float>, <64 x float> }
@i32_v64pair = external dso_local addrspace(2) global %struct._int64_pair_t, section ".vldata"

; Function Attrs: noinline nounwind
define dso_local void @main() #0 {
entry:
  %fp_source = bitcast %struct._float64_pair_t addrspace(2)* inttoptr (i32 0 to %struct._float64_pair_t addrspace(2)*) to <128 x float> addrspace(2)*
  %0 = load <128 x float>, <128 x float> addrspace(2)* %fp_source, align 256

  %1 = fptosi <128 x float> %0 to <128 x i32>

  %s1 = shufflevector <128 x i32> %1, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %i32_v64pair.a = getelementptr inbounds %struct._int64_pair_t, %struct._int64_pair_t addrspace(2)* inttoptr (i32 0 to %struct._int64_pair_t addrspace(2)*), i32 0, i32 0
  store <64 x i32> %s1, <64 x i32> addrspace(2)* %i32_v64pair.a, align 256

  %s2 = shufflevector <128 x i32> %1, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %i32_v64pair.b = getelementptr inbounds %struct._int64_pair_t, %struct._int64_pair_t addrspace(2)* inttoptr (i32 0 to %struct._int64_pair_t addrspace(2)*), i32 0, i32 1
  store <64 x i32> %s2, <64 x i32> addrspace(2)* %i32_v64pair.b, align 256

  ret void
}

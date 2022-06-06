#ifndef LLVM_TPCMACHINELOOPDEPENDENCYANALYSIS_H
#define LLVM_TPCMACHINELOOPDEPENDENCYANALYSIS_H

#include "TPCMachineLoopLinearIV.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallPtrSet.h"

namespace llvm {
class MachineInstr;
class MachineLoop;
class MachineRegisterInfo;
class TPCInstrInfo;
class TPCSubtarget;

/// Machine Loop Dependency Analysis Report.
///
/// \note Due to restricted dependency analysis functionality dependency might
///       be reported if it actually absent for some inputs. For example,
///       dependency might exist but iterator value which triggers dependency is
///       outside loop boundaries. Such dependencies will be reported but never
///       happen.
///
/// \note Another way of understanding Optional<bool> HasDependency property is:
///       - true  - analysis has detected a dependency in some circumstances.
///       - false - analysis guarantees there is no dependency.
///       - None  - analysis failed, it can not say anything on dependencies.
struct MLDAReport {
  Optional<bool> HasLoopDependentReg;
  Optional<bool> HasLoopDependentMemStLd;
  Optional<bool> HasLoopDependentMemStSt;
  Optional<bool> HasLoopIndependentMemStLd;
  Optional<bool> HasLoopIndependentMemStSt;

  /// Phi instructions which define registers produced loop dependency.
  ///
  /// \details Container is not empty iff HasLoopDependentReg is true. Some
  ///     passes allow optimizations for some kinds of loop dependencies in
  ///     registers. For example, accumulators like array reduction by sum,
  ///     dot product, min/max search ... infer loop dependent dependencies
  ///     in registers, but unrolling pass might transform accumulators and
  ///     ignore this instructions.
  SmallPtrSet<MachineInstr *, 8> PhisDefineLoopDependentReg;
};

/// Analyze machine loop for loop-dependent data dependencies.
///
/// \details If dependencies with distance >= InterestingIterationsCount are
///     not interested for caller, analysis might provide more precise result.
///     For example, dependencies with distance >= unroll count are not
///     interesting for unrolling pass. Leave InterestingIterationsCount value
///     default to search for dependencies without distance limitation.
MLDAReport runMachineLoopDataDependencyAnalysis(
    const MachineLoop &ML, const MachineRegisterInfo &MRI,
    const TPCSubtarget &ST, const TPCInstrInfo &TII,
    const MLLinearIVsContainer &LinearIVs,
    bool IgnoreCheckAgainstStoresToSameTensorId,
    unsigned InterestingIterationsCount = 0);

} // namespace llvm

#endif // LLVM_TPCMACHINELOOPDEPENDENCYANALYSIS_H

#ifndef LLVM_TPCMACHINELOOPLINEARIV_H
#define LLVM_TPCMACHINELOOPLINEARIV_H

#include "TPCLoopData.h"

#include "llvm/ADT/Optional.h"
#include "llvm/CodeGen/Register.h"

namespace llvm {
class MachineInstr;
class MachineLoop;
class MachineOperand;
class MachineRegisterInfo;
class raw_ostream;

/// Info about custom induction variable of machine loop.
///
/// \details Custom IV is defined with pair of machine instructions: Phi and
///     step definition. It is something like that:
///
///     \code
///     %curr_it = PHI [%initial_value, PredMBB], [%next_it, LatchMBB]
///     %next_it = ADD %curr_it, %step
///     \endcode
///
struct MLoopCustomLinearIVInfo {
  /// Phi which defines the current iteration value.
  MachineInstr *Phi;

  /// Instruction which defines the next iteration value.
  MachineInstr *DefMI;

  /// Index of step operand in DefMI.
  ///
  /// \note Might be not immediate.
  unsigned StepOpIx;
};

/// Umbrella class to represent machine loop linear induction variable.
class MLoopLinearIV {
public:
  /// Create from custom IV.
  explicit MLoopLinearIV(const MLoopCustomLinearIVInfo Info);

  /// Create from hw loop counter.
  explicit MLoopLinearIV(const HWLoopCounterInfo Info);

  /// Whether IV is custom.
  bool isCustom() const;

  /// Whether IV is hw loop counter (S32, S33 ..).
  bool isHWCounter() const;

  /// Return instruction which defines IV value on the current iteration.
  MachineInstr &getCurrDefMI() const;

  /// Return instruction which defines IV value on the next iteration.
  MachineInstr &getNextDefMI() const;

  /// Return register with definition of IV value on the current iteration.
  Register getCurrDefReg() const;

  /// Return register with definition of IV value on the next iteration.
  Optional<Register> getNextDefReg() const;

  /// Return index of the step operand in step appliance instruction.
  unsigned getStepOpIx() const;

  /// Return index of the curr value operand in step appliance instruction.
  /// Makes sense only for custom IVs.
  Optional<unsigned> getCurrValOpIxInNextDefMI() const;

  /// Shortcut to get step operand.
  MachineOperand &getStepOp() const;

  /// Return operand which defines IV start value.
  ///
  /// \details Might return nullptr if there are several start value operands.
  ///     For example, loop header Phi instruction might have several inputs
  ///     from MBBs outside loop. All of them might define different IV start
  ///     value. In this case there is no single start value machine operand.
  MachineOperand *getStartOp() const;

private:
  friend raw_ostream &operator<<(raw_ostream &OS, const MLoopLinearIV &Info);

  // Implementation via union is chosen in order to avoid inheritance and
  // required memory allocations then, but it is not perfect C++ taste.
  //
  // Note that std::variant would be much cleaner implementation than tagged
  // union but std::variant is available since C++17 only.
  union ContentUnion {
    MLoopCustomLinearIVInfo CustomIVInfo;
    HWLoopCounterInfo CounterInfo;
  } Content;
  int ContentFlag;
};

using MLLinearIVsContainer = SmallVector<MLoopLinearIV, 4>;

/// Find linear induction variables in machine loop.
///
/// \note Function is not required to return all induction variables, sometimes
///       loops are too complex to analyze. But it tries its best.
MLLinearIVsContainer findLinearIVs(const MachineLoop &ML,
                                   const MachineRegisterInfo &MRI);

raw_ostream &operator<<(raw_ostream &OS, const MLoopCustomLinearIVInfo &Info);
raw_ostream &operator<<(raw_ostream &OS, const HWLoopCounterInfo &Info);
raw_ostream &operator<<(raw_ostream &OS, const MLoopLinearIV &Info);

} // namespace llvm

#endif // LLVM_TPCMACHINELOOPLINEARIV_H

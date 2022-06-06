//===----------------------------------------------------------------------===//
//
// Utility functions for TPC MachineInstructions processing.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TPCMACHINEINSTRTOOLS_H
#define LLVM_TPCMACHINEINSTRTOOLS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineOperand.h"

#include <cstdint>

namespace llvm {
class MachineRegisterInfo;
class TargetInstrInfo;

/// Return Opcode for LOOP instruction based on params properties.
///
/// For example, for non-predicated loop with immediate operands function
/// returns TPC::LOOPiii value.
unsigned getLoopMachineInstrOpcode(const bool IsStartImm, const bool IsBoundImm,
                                   const bool IsStepImm,
                                   const bool IsPredicated);

/// Shortcut to evaluate SUB for I32.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instruction and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalSubI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS,
                          const MachineOperand &OpRHS, MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII);

/// Shortcut to evaluate ADD for I32.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instruction and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalAddI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS,
                          const MachineOperand &OpRHS, MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII);

/// Shortcut to evaluate SHR - unsigned shift right for I32.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instructions and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalShrI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS, const std::int32_t OpRHS,
                          MachineRegisterInfo &MRI, const TargetInstrInfo &TII);

/// Shortcut to evaluate SHRS - signed shift right for I32.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instructions and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalShrSI32(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator InsertPos,
                           const MachineOperand &OpLHS,
                           const std::int32_t OpRHS, MachineRegisterInfo &MRI,
                           const TargetInstrInfo &TII);

/// Shortcut to evaluate SHL - shift left for I32.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instructions and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalShlI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS, const std::int32_t OpRHS,
                          MachineRegisterInfo &MRI, const TargetInstrInfo &TII);

/// Shortcut to evaluate bitwise AND for I32.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instructions and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalAndI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS,
                          const MachineOperand &OpRHS, MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII);

/// Shortcut to evaluate REM for positive immediate I32 denominator.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instructions and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalRemPosI32(MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator InsertPos,
                             const MachineOperand &OpLHS,
                             const std::int32_t OpRHS, MachineRegisterInfo &MRI,
                             const TargetInstrInfo &TII);

/// Shortcut to evaluate REM for positive immediate I32 denominator and
/// guaranteed positive I32 numerator even if numerator is a register.
///
/// Returns immediate operand if value is known at compile time, otherwise
/// inserts calculation instructions and returns register operand. Note that
/// returned operand is just a register/constant holder.
MachineOperand evalRemPosToPosI32(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator InsertPos,
                                  const MachineOperand &OpLHS,
                                  const std::int32_t OpRHS,
                                  MachineRegisterInfo &MRI,
                                  const TargetInstrInfo &TII);

/// Shortcut to insert implicit register definition instruction for the given
/// register class.
Register insertImplicitDef(const TargetRegisterClass &RC,
                           MachineBasicBlock &MBB,
                           const MachineBasicBlock::iterator Before,
                           MachineRegisterInfo &MRI,
                           const TargetInstrInfo &TII);

/// Shortcut to insert COPY instruction for the given virtual register.
Register insertVRegCOPY(const Register SrcReg, MachineBasicBlock &MBB,
                        const MachineBasicBlock::iterator Before,
                        MachineRegisterInfo &MRI, const TargetInstrInfo &TII);

/// Look for phi input value for the given MBB.
MachineOperand *getPhiValueForMBB(MachineInstr &Phi,
                                  const MachineBasicBlock &MBB);
const MachineOperand *getPhiValueForMBB(const MachineInstr &Phi,
                                        const MachineBasicBlock &MBB);

/// Helper to organize an input pair of phi instruction.
struct PhiIP {
  MachineOperand *ValOp = nullptr; /// Input value.
  MachineOperand *MBBOp = nullptr; /// Related machine basic block.
};

/// Get all phi inputs.
SmallVector<PhiIP, 4> getAllPhiIPs(MachineInstr &Phi);

/// Find input pair in phi instruction which corresponds to the given MBB.
Optional<PhiIP> getPhiIPForMBB(MachineInstr &Phi, const MachineBasicBlock &MBB);

/// Check whether phi has given inputs only.
bool isPhiInputs(const MachineInstr &Phi, ArrayRef<const PhiIP*> Inputs);

/// Removes phi operands which denotes input from the given MBB.
bool removePhiInputFromMBB(MachineInstr& Phi, const MachineBasicBlock& MBB);

/// Return index of phi operand which denotes the given machine basic block.
Optional<unsigned> getIndexOfPhiMBBOp(const MachineInstr &Phi,
                                      const MachineBasicBlock &MBB);

/// Return index of phi operand which denotes the input value from the given
/// machine basic block.
Optional<unsigned> getIndexOfPhiValOp(const MachineInstr &Phi,
                                      const MachineBasicBlock &MBB);

/// Insert phi instruction which defines register of class RC and accepts the
/// given IPs inputs.
MachineInstr &insertPHI(const TargetRegisterClass &RC,
                        ArrayRef<const PhiIP *> IPs, MachineBasicBlock &MBB,
                        const MachineBasicBlock::iterator Before,
                        MachineRegisterInfo &MRI, const TargetInstrInfo &TII);

/// Return whether register machine operand is likely tied to another register
/// and it leads to physical registers live range union after register
/// allocator pass.
///
/// \details MO.isTied() check is not enough to ensure that register will be
///     tied to its counterpart. Function checks some tricky knowledge which
///     allows to filter cases when MO.isTied() returns true, but register
///     allocator will not actually tie physical registers.
///
/// \return False if register allocator will not tie two registers into physical
///     one. True if register allocator might tie them.
bool isLikelyTied(const MachineOperand &MO, const MachineRegisterInfo &MRI);

/// Return whether machine operand is register and is a placeholder. I.e. there
/// is no actual virtual register usage and physical register occupation.
bool isPlaceholderReg(const MachineOperand &MO, const MachineRegisterInfo &MRI);

} // namespace llvm

#endif // LLVM_TPCMACHINEINSTRTOOLS_H

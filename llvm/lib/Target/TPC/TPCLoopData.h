//===------------------------------LoopData--------------------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2019 - This header is a property of Habana labs
//
//
//===----------------------------------------------------------------------===//
//                                 LoopData
// LoopData extract inforamtion on the Loop by asking SCEV on the Loop SCALAR. 
//===----------------------------------------------------------------------===//

#ifndef LLVM_TPCLOOPDATA_CPP_H
#define LLVM_TPCLOOPDATA_CPP_H
#include "TPCTargetMachine.h"
#include "SCEVParser.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

namespace llvm {
class MDNode;
class MachineInstr;
class MachineLoop;

/// Check if loop meta data node has "llvm.loop.taken" sub node.
bool hasLoopTakenMD(const MDNode &LoopMD);

/// Remove "llvm.loop.taken" meta data sub node from LOOPEND instruction if any.
void removeLoopTakenMD(MachineInstr &LoopEndMI);

/// Return meta data node for machine loop if exists.
const MDNode *getMachineLoopMDNode(MachineInstr &LoopEndMI);

/// Shortcut to find meta data sub node by name for loop metas.
const MDNode* findOptionMDForLoopMD(const MDNode& LoopMD, StringRef Name);

/// Returns unroll count preference from meta data.
Optional<unsigned> getUnrollCountFromMetadata(const MDNode *LoopMD);

/// Helper struct to organize result of HW loop counter search.
///
/// \details HW loop counter is a physical register defined by LOOP instruction.
struct HWLoopCounterInfo {
  MachineInstr *LoopInstr;
  unsigned CounterReg;
};

/// Get counter register for HW loop.
///
/// \details According to TPC Programming Reference Guide the top most parent
///     loop uses S32 register. Inner loops use S33, S34 and S35 registers
///     corresponding to their nesting level.
Register getCounterRegister(const MachineLoop &ML);

/// Get information about hardware loop counter if applicable.
Optional<HWLoopCounterInfo> getLoopCounterInfo(const MachineLoop &ML);

} // namespace llvm

class LoopData {
public:
  LoopData(){};
  LoopData(Loop *L, ScalarEvolution *SE, bool costModel = false);
  std::pair<Instruction *, Instruction *> getCmpValue();
  unsigned get_STEP() { return m_STEP; }
  unsigned get_DIM() { return m_DIM; }
  const SCEV *getLoopSCEV() { return p_LoopSCEV; }
  bool getSCEVStatus() { return m_SCEVNotValid; }
  bool is_Valid() { return m_Valid; }

private:
  Module *p_MD;
  Loop *p_LH;
  Loop *p_Prev;
  BasicBlock *p_Nested;
  BasicBlock *p_Latch;
  ScalarEvolution *p_SEL;
  Instruction *p_Inducation = nullptr;
  unsigned m_STEP = 0;
  unsigned m_DIM = 0;
  bool m_Valid = false;
  bool m_SCEVNotValid = true;
  unsigned m_backendUnroll = 1;
  const SCEV *p_LoopSCEV;

  const SCEV *tryFindSCEV();
  void findNumberOfIterations(BasicBlock *BB);
  std::pair<IntrinsicInst *, IntrinsicInst *> findOffsetAndSizeIntrinsics();
  const SCEV *relaxSCEV(const SCEV *EV, vector<const Instruction *> index,
                        string name);
};

#endif // LLVM_TPCLOOPDATA_CPP_H

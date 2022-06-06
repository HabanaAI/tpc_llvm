//===---- TPCHardwareLoops.cpp - Identify and generate hardware loops -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass identifies loops where we can generate the TPC hardware
// loop instruction.
//
// Criteria for hardware loops:
//  - Countable loops (w/ ind. var for a trip count)
//  - Try inner-most loops first
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCLoopData.h"
#include "TPCMachineInstrTools.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <map>
#include <set>
#include <utility>
#include <vector>
using namespace llvm;

#define DEBUG_TYPE "hwloops"

static cl::opt<bool>
    RemoveJumps("remove-jumps", cl::Hidden,
                    cl::desc("Remove jumps before HW loops"), cl::init(false));

// Default limit is chosen based on code base under testing and might be
// fine-tuned later.
static cl::opt<unsigned> MaxHWLoopInstructionsCount(
    "max-hw-loop-inst-count", cl::Hidden, cl::init(2500), cl::Optional,
    cl::desc(
        "Maximum number of instructions in machine loop to transform it "
        "into hardware form. Larger loops will not be transformed. Hardware "
        "has restriction on hw loop body size. Tune this limit to avoid 'Too "
        "many instructions in the LOOP' compilation error on asm printing "
        "stage."));

namespace llvm {

  FunctionPass *createTPCHardwareLoops();
  void initializeTPCHardwareLoopsPass(PassRegistry&);

} // end namespace llvm
static const char PassDescription[] = "TPC Hardware Loops";
static const char PassName[] = "tpc-hwloops";
static cl::opt<bool>
EnableTPCHardwareLoops(PassName,
  cl::Hidden,
  cl::init(true));

static unsigned getUserSpecifiedLoopHWUnrollCount(MachineLoop &ML) {
  MachineBasicBlock *LatchMBB = ML.getLoopLatch();

  // Loops without latch can not be unrolled (see TPCSoftwarePipelining
  // preconditions)
  if (!LatchMBB || !LatchMBB->getBasicBlock())
    return 1;

  MDNode *LoopMD = LatchMBB->getBasicBlock()->getTerminator()->getMetadata(
      LLVMContext::MD_loop);
  return getUnrollCountFromMetadata(LoopMD).getValueOr(1);
}

/// Predict loop instructions count at the end of optimization pipeline.
///
/// \details It is workaround for problem that hardware does not support
///     hardware loops with too big instructions count. Alas, knowledge about
///     further passes details is leaked here. Leaked knowledge is a hotfix and
///     should be removed when TPCUnHardwareLoops pass is completed.
static unsigned
getLoopInstructionsCountPrediction(const MachineLoop &ML,
                                   const MachineLoopInfo &MLI,
                                   const TargetSubtargetInfo &TST) {
  const FeatureBitset& Features = TST.getFeatureBits();

  // "TPC Hardware Workarounds for converts"-specific tricks.
  const bool IsTPCHardwareWorkaroundForConvertIntsRequired =
      Features[TPC::FeatureGreco] || Features[TPC::FeatureGaudi2];
  const bool IsTPCHardwareWorkaroundForConvertRequired =
      !IsTPCHardwareWorkaroundForConvertIntsRequired;

  unsigned Count = 0;
  for (const MachineBasicBlock *MBB : ML.getBlocks()) {
    MachineLoop *MBBLoop = MLI.getLoopFor(MBB);
    assert(MBBLoop); // At least ML must be returned.

    const unsigned UnrollCount = getUserSpecifiedLoopHWUnrollCount(*MBBLoop);
    for (const MachineInstr& MI : *MBB) {
      const unsigned Opcode = MI.getOpcode();

      unsigned ToAdd = 1;
      // "TPC Hardware Workarounds for converts" pass might add up to 5
      // instructions for each CONVERT_INT
      if (IsTPCHardwareWorkaroundForConvertIntsRequired &&
          (Opcode == TPC::CONVERT_INT32g3i8vip ||
           Opcode == TPC::CONVERT_UINT32g3i8vip ||
           Opcode == TPC::CONVERT_INT32g3i8vim ||
           Opcode == TPC::CONVERT_UINT32g3i8vim ||
           Opcode == TPC::CONVERT_INT32g3i8vsp ||
           Opcode == TPC::CONVERT_UINT32g3i8vsp ||
           Opcode == TPC::CONVERT_INT32g3i8vsm ||
           Opcode == TPC::CONVERT_UINT32g3i8vsm ||
           Opcode == TPC::CONVERT_INT32g3i16vip ||
           Opcode == TPC::CONVERT_UINT32g3i16vip ||
           Opcode == TPC::CONVERT_INT32g3i16vim ||
           Opcode == TPC::CONVERT_UINT32g3i16vim ||
           Opcode == TPC::CONVERT_INT32g3i16vsp ||
           Opcode == TPC::CONVERT_UINT32g3i16vsp ||
           Opcode == TPC::CONVERT_INT32g3i16vsm ||
           Opcode == TPC::CONVERT_UINT32g3i16vsm ||
           Opcode == TPC::CONVERT_INT16g3Avip ||
           Opcode == TPC::CONVERT_UINT16g3Avip ||
           Opcode == TPC::CONVERT_INT16g3Avim ||
           Opcode == TPC::CONVERT_UINT16g3Avim ||
           Opcode == TPC::CONVERT_INT16g3Avsm ||
           Opcode == TPC::CONVERT_UINT16g3Avsm ||
           Opcode == TPC::CONVERT_INT16g3Avsp ||
           Opcode == TPC::CONVERT_UINT16g3Avsp))
        ToAdd += 5;
      // "TPC Hardware Workarounds for converts" pass might add up to 2
      // instructions for each CONVERTvvp / CONVERTssp
      if (IsTPCHardwareWorkaroundForConvertRequired &&
          (Opcode == TPC::CONVERTvvp || Opcode == TPC::CONVERTssp))
        ToAdd += 2;

      Count += ToAdd * UnrollCount;
    }
  }

  // Apply 2x factor for instructions count to compensate code bloat from
  // "Two-Address instruction pass" and "Pipeline for same register" passes.
  Count *= 2;

  return Count;
}

static bool isHWLoopTransformationAllowed(const MachineLoop &ML,
                                          const MachineLoopInfo &MLI,
                                          const TargetSubtargetInfo &TST) {
  // LOOP hardware instruction supports at most 2**16 address offset. It means
  // that encoded loop body must fit into 2**16 bytes. Compiler will report
  // failure for longer loops.
  //
  // To deal with that issue let's provide a heuristic: how large the loop is.
  // Disallow hardware loop transformation for too long loops. It should not
  // affect performance much because loop administration overhead is small
  // for large loops, but it should fix some unexpected compilation failures.

  // Instruction compression should decrease code size ~2x times. Use heuristic
  // for compression iff user is not specified limit manually.
  const unsigned Factor = MaxHWLoopInstructionsCount.isDefaultOption() &&
                                  ML.getHeader()
                                      ->getParent()
                                      ->getSubtarget<TPCSubtarget>()
                                      .getTargetLowering()
                                      ->getTargetMachine()
                                      .Options.CompressInstructions
                              ? 2
                              : 1;
  return getLoopInstructionsCountPrediction(ML, MLI, TST) <=
         MaxHWLoopInstructionsCount * Factor;
}

namespace {
struct LoopVars {
  union {
    unsigned Reg;
    int64_t ImmVal;
  } Start;

  union {
    unsigned Reg;
    int64_t ImmVal;
  } Boundary;

  union {
    unsigned Reg;
    int64_t ImmVal;
  } Step;

  bool StartSel = false;
  bool BoundarySel = false;
  bool StepSel = false;

  unsigned PredReg = 0;
  unsigned Polarity = 0;
  bool IsPredicated = false;
  bool IsTaken = false;

  TPCII::CmpMode Mode = TPCII::LoopErr;

  bool isStart(MachineOperand& MO) const {
    return StartSel
        ? MO.isImm() && MO.getImm() == Start.ImmVal
        : MO.isReg() && MO.getReg() == Start.Reg;
  }

  bool isBoundary(MachineOperand& MO) const {
    return BoundarySel
        ? MO.isImm() && MO.getImm() == Boundary.ImmVal
        : MO.isReg() && MO.getReg() == Boundary.Reg;
  }

  bool isStep(MachineOperand& MO) const {
    return StepSel
        ? MO.isImm() && MO.getImm() == Step.ImmVal
        : MO.isReg() && MO.getReg() == Step.Reg;
  }

  void dump() {
    dbgs() << "Start is ";
    if (StartSel) {
      dbgs() << Start.ImmVal << "\n";
    } else {
      dbgs() << "%vreg" << Start.Reg << "\n";
    }

    dbgs() << "Boundary is ";
    if (BoundarySel) {
      dbgs() << Boundary.ImmVal << "\n";
    } else {
      dbgs() << "%vreg" << Boundary.Reg << "\n";
    }

    dbgs() << "Step is ";
    if (StepSel) {
      dbgs() << Step.ImmVal << "\n";
    } else {
      dbgs() << "%vreg" << Step.Reg << "\n";
    }

    dbgs() << "PredReg is " << PredReg << "\n";
    dbgs() << "Polarity is " << Polarity << "\n";
    dbgs() << "IsPredicated is " << IsPredicated << "\n";
    dbgs() << "IsTaken is " << IsTaken << "\n";
    dbgs() << "Cmp mode is " << Mode << "\n";
  }
};

struct LoopStats {
  unsigned Phi;
  unsigned Iter;
  unsigned CounterReg;
  union {
    unsigned Reg;
    int64_t ImmVal;
  } Step;
  bool ImmStep;
  MachineLoop* L;
  MachineInstr *LoopMI;
  MachineInstr *LoopendMI;


  LoopStats(unsigned Phi_, unsigned Iter_, MachineLoop* L_) : Phi(Phi_),
    Iter(Iter_), CounterReg(0), ImmStep(false), L(L_) {}
};

class TPCHardwareLoops : public MachineFunctionPass {
  MachineLoopInfo            *MLI;
  MachineRegisterInfo        *MRI;
  MachineDominatorTree       *MDT; //Needed?
  const TPCSubtarget         *TST;
  const TPCInstrInfo         *TII;

  SmallVector<MachineOperand, 8> Iterators;
  std::map<MachineLoop*, LoopStats> LoopOrder;
  std::map<unsigned, SmallVector<LoopStats, 4>> RegToLoop;
  SmallVector<MachineBasicBlock *, 8> RemoveList;

public:
  static char ID;
  TPCHardwareLoops() : MachineFunctionPass(ID), MLI(nullptr), MRI(nullptr),
                       MDT(nullptr), TII(nullptr) {
    initializeTPCHardwareLoopsPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool convertToHardwareLoop(MachineLoop *L, int& Layer);
  void setCounters(MachineLoop* L, int& Layer);
  llvm::Optional<LoopVars> calculateLoop(MachineLoop *L);

  StringRef getPassName() const override { return "TPC Hardware Loops"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  bool checkLayout(MachineLoop *L);
  TPCII::CmpMode  getCmpMode(unsigned Opcode) const;
  TPCII::CmpMode  invertCond(TPCII::CmpMode mode) const;
  TPCII::CmpMode  swapCond(TPCII::CmpMode mode) const;
  unsigned chooseLoopOpcode(LoopVars lv) const;
  MachineBasicBlock* stripEntranceCheck(MachineBasicBlock* Preheader, MachineBasicBlock* Header, MachineBasicBlock* Exit, LoopVars* Lv);
  bool predicateLoop(MachineBasicBlock* Preheader, MachineBasicBlock* Exit, MachineBasicBlock* PredTarget, LoopVars* Lv);
  void replaceIteratorsWithLoopCounter();
  void addMissingEdges();
};
}

INITIALIZE_PASS_BEGIN(TPCHardwareLoops, PassName,
                      PassDescription, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(TPCHardwareLoops, PassName,
                    PassDescription, false, false)

FunctionPass *llvm::createTPCHardwareLoops() {
  return new TPCHardwareLoops();
}

bool TPCHardwareLoops::checkLayout(MachineLoop *L) {
  MachineBasicBlock* Latch = L->getLoopLatch();
  MachineBasicBlock* Exit = L->getExitBlock();

  assert(Exit && "We should've checked earlier that the loop has only one exit");

  // There's a pass that set numbers to blocks in order they will be emitted.
  // But it's launched much later. In theory we can bring it before loop conversion
  // by adding it to the dependency list. But for now it looks like we don't need
  // such a strict check
//  int StartNum = Start->getNumber();
//  int EndNum = Latch->getNumber();
//  int ExitNum = Exit->getNumber();
//
//  if (ExitNum < StartNum)
//    return false;
//
//  if (EndNum < StartNum)
//    return false;
//
//  for (int i = StartNum; i < EndNum; ++i) {
//    MachineBasicBlock* NextLayout = Start->getParent()->getBlockNumbered(i);
//    // The loop should be layouted contiguously.
//    if (!L->contains(NextLayout)) {
//      return false;
//    }
//  }

  // The loop exit should be layouted just after the loop
  if (!Latch->isLayoutSuccessor(Exit)) {
    return false;
  }

  if (!L->isLoopExiting(Latch)) {
    return false;
  }

  int OnlyOneExit = 0;
  for (auto B : L->getBlocks()) {
    if (L->isLoopExiting(B)) OnlyOneExit++;
    if (OnlyOneExit > 1) return false;
  }

  return true;
}

namespace {
  // Helper to organize phi instruction data inside loop when one phi input
  // comes from basic block outside loop and another one input from basic block
  // inside loop.
  struct LoopEIPhiInputs {
    PhiIP External;  // Input from basic block outside loop.
    PhiIP Internal;  // Input from basic block inside loop.
  };
}  // namespace

// Split phi instruction inside loop input pairs on internal (from blocks
// inside loop) and external (from blocks outside of the loop). Return None if
// phi inputs are only from basic blocks inside loop.
static llvm::Optional<LoopEIPhiInputs>
GetLoopEIPhiInputs(MachineInstr &Phi, MachineLoop &L) {
  assert(Phi.isPHI() && "precondition: Phi must be a phi instruction");
  assert(L.contains(&Phi) && "precondition: Phi must be inside loop L");

  LoopEIPhiInputs Res;
  for (int Ix = 1, Count = Phi.getNumOperands(); Ix < Count; Ix += 2) {
    const PhiIP P{&Phi.getOperand(Ix), &Phi.getOperand(Ix + 1)};
    (L.contains(P.MBBOp->getMBB()) ? Res.Internal : Res.External) = P;
  }

  if (!Res.External.ValOp || !Res.Internal.ValOp)
    return llvm::None;
  return Res;
}

// Return whether instruction op code fits for loop incerement definition like:
//   <next_it> = ADD <curr_it>, <step>
static bool isIncrement(const unsigned Opcode) {
  return Opcode == TPC::ADDssp || Opcode == TPC::ADDsip;
}

// Loop patterns parsed by hwloop pass
enum class HWLoopPattern {
  // Loops with comparator on increment result:
  //
  //   <curr_it> = PHI <init_value>, <pre_block>, <next_it>, <latch_block>
  //   ...
  //   <next_it> = ADD <curr_it>, <step>
  //   ...
  //   <loop_pred> = CMP_* <next_it>, <boundary>  !!! NOTE: next_it here
  //   ...
  //   JMPR <header_block>, <loop_pred>
  PostIncrement,

  // Loops with comparator on increment source:
  //
  //   <curr_it> = PHI <init_value>, <pre_block>, <next_it>, <latch-block>
  //   ...
  //   <next_it> = ADD <curr_it>, <step>
  //   ...
  //   <loop_pred> = CMP_* <curr_it>, <boundary>  !!! NOTE: curr_it here
  //   ...
  //   JMPR <header_block>, <loop_pred>
  PreIncrement
};

llvm::Optional<LoopVars> TPCHardwareLoops::calculateLoop(MachineLoop *L) {
  MachineBasicBlock *Header = L->getHeader();
  MachineBasicBlock *Latch = L->getLoopLatch();
  MachineBasicBlock *ExitingBlock = L->findLoopControlBlock();
  MachineBasicBlock* ExitBlock = L->getExitBlock();

  LLVM_DEBUG(dbgs() << "Calculate loop\n");

  if (!Header)
    return llvm::None;

  // Loops with several backedges can't be converted
  if (!Latch)
    return llvm::None;

  // Don't generate hw loop if the loop has more than one exit.
  if (!ExitingBlock)
    return llvm::None;

  if (!ExitBlock)
    return llvm::None;

  if (!checkLayout(L))
    return llvm::None;

  MachineOperand* Pred = nullptr;
  bool InvertCond = false;
  MachineInstr* Terminator = nullptr;
  for (auto I = Latch->instr_rbegin(); I != Latch->instr_rend(); ++I) {
    Terminator = &*I;

    if (Terminator->getOpcode() == TPC::JMPR) {
      //This is the backedge that we need. Extract predicate from it.
      Pred = &Terminator->getOperand(1);
      const bool IsDestinationBlockInLoop =
          L->contains(Terminator->getOperand(0).getMBB());
      InvertCond = Terminator->getOperand(2).getImm() != 0
          ? IsDestinationBlockInLoop
          : !IsDestinationBlockInLoop;
      break;
    }
  }

  // If there's no conditional jump inside the loop, it's a precondition loop
  if (!Pred)
    return llvm::None;

  MachineInstr* Condition = MRI->getVRegDef(Pred->getReg());
  assert(Condition && "Multidef of a predicate in SSA form");
  if (Condition->isCopy()) {
    Condition = MRI->getVRegDef(Condition->getOperand(1).getReg());
  }

  if (getCmpMode(Condition->getOpcode()) == TPCII::LoopErr) {
    // Skip loop if predicate is calculated as AND of several conditions
    return llvm::None;
  }

  if (!L->contains(Condition)) {
    return llvm::None;
  }
  assert(L->contains(Condition) && "Condition for backedge is not in the loop");

  LLVM_DEBUG(dbgs() << "Found condition:\n");
  LLVM_DEBUG(Condition->dump());
  LLVM_DEBUG(dbgs() << "\n");

  // Get iterator and boundary from CMP instruction.
  MachineOperand* Iterator = nullptr;
  MachineOperand* Boundary = nullptr;
  bool NeedSwapCondition = false;
  MachineOperand& Op1 = Condition->getOperand(1);
  MachineOperand& Op2 = Condition->getOperand(2);
  if (!Op1.isReg()) {
    Boundary = &Op1;
    Iterator = &Op2;
    NeedSwapCondition = true;
  } else if (!Op2.isReg()) {
    Boundary = &Op2;
    Iterator = &Op1;
  } else if (Op1.isReg() && Op2.isReg()) {
    // Shared iterator between loops
    if (!MRI->getVRegDef(Op1.getReg()) || ! MRI->getVRegDef(Op2.getReg())) {
      return llvm::None;
    }
    if (!L->contains(MRI->getVRegDef(Op1.getReg()))) {
      if (!L->contains(MRI->getVRegDef(Op2.getReg()))) {
        return llvm::None;
      }
      Iterator = &Op2;
      Boundary = &Op1;
      NeedSwapCondition = true;
    } else if (!L->contains(MRI->getVRegDef(Op2.getReg()))) {
      if (!L->contains(MRI->getVRegDef(Op1.getReg()))) {
        return llvm::None;
      }
      Iterator = &Op1;
      Boundary = &Op2;
    } else {
      return llvm::None;
    }

    MachineInstr* BoundaryDef = MRI->getVRegDef(Boundary->getReg());
    if (L->contains(BoundaryDef)) {
      // TODO: If at this point loop invariants are not moved out of the loop
      // we should check wether boundary def is a real invariant and move it
      // ourselfs. If it's not possible, the loop can't be converted.
      return llvm::None;
    }
  }

  if (!Iterator || !Boundary) {
    LLVM_DEBUG(dbgs() << "Failed to make hw loop: condition is not parsed\n");
    return llvm::None;
  }
  LLVM_DEBUG(dbgs() << "Boundary from condition: "; Boundary->dump());
  LLVM_DEBUG(dbgs() << "Iterator from condition: "; Iterator->dump());

  // Search for the iterator increment.
  //
  // For supported loop patterns CmpUseDef is:
  //   either: <next_it> = ADD ... (PostIncrement)
  //   either: <curr_it> = PHI ... (PreIncrement)
  MachineInstr* const CmpUseDef = MRI->getVRegDef(Iterator->getReg());
  LLVM_DEBUG(dbgs() << "Iterator def used in CMP: ");
  LLVM_DEBUG(CmpUseDef->dump());
  assert(L->contains(CmpUseDef) && "Iterator increment is not in the loop");

  // Locate machine instructions and operands responsible for iteration
  MachineInstr *StepInstr = nullptr;  // ADD instruction for loop step
  int64_t StepImmInc = 0;  // Increment for loop step immediate value
  MachineInstr *Phi = nullptr;
  HWLoopPattern Pattern;
  if (isIncrement(CmpUseDef->getOpcode())) {
    // Likely a PostIncrement pattern
    Pattern = HWLoopPattern::PostIncrement;
    LLVM_DEBUG(dbgs() << "Process PostIncrement pattern\n");

    StepInstr = CmpUseDef;
    LLVM_DEBUG(dbgs() << "StepInstr: "; StepInstr->dump());

    // TODO: Below I assume that step is the second operand and iterator is
    //       the first one. This is not necessary true.
    const MachineOperand& PhiOp = StepInstr->getOperand(1);
    assert(PhiOp.isReg() && "Iterator can't be a constant");
    Phi = MRI->getVRegDef(PhiOp.getReg());
    LLVM_DEBUG(dbgs() << "Step instruction operand def: "; Phi->dump());

    // Try to recognize chain of ADD instructions with imm step:
    //   %1 = PHI ... [%n, BB.Latch]
    //   %2 = ADD %1, c1
    //   %3 = ADD %2, c2
    //   ...
    //   %n = ADD %n-1, c_n-1
    if (!Phi->isPHI()) {
      if (!StepInstr->getOperand(2).isImm())
        return llvm::None;

      while(!Phi->isPHI()) {
        if (!TPCII::isAdd(Phi->getDesc())) {
          return llvm::None;
        }

        if (Phi->getOperand(2).isImm()) {
          StepImmInc += Phi->getOperand(2).getImm();
          Phi = MRI->getVRegDef(Phi->getOperand(1).getReg());
        } else if (Phi->getOperand(1).isImm()) {
          StepImmInc += Phi->getOperand(1).getImm();
          Phi = MRI->getVRegDef(Phi->getOperand(2).getReg());
        } else {
          return llvm::None;
        }
      }
    }
    LLVM_DEBUG(dbgs() << "Phi node: "; Phi->dump());
    assert(Phi->isPHI() && "Iterator should be a phi node");

    if (!L->contains(Phi)) {
      LLVM_DEBUG(dbgs() << "Failed to make hw loop: "
                           "start phi node is out of the loop\n");
      return llvm::None;
    }

    // Some sort of double inductive variable. We can't link iterator increment
    // with initial increment value through a phi node.
    // TODO: it still may be possible to form hw loop, just more clever
    //       analysis is required
    const bool IVinPhi = llvm::any_of(Phi->uses(), [=](MachineOperand &MUse) {
                           return MUse.isIdenticalTo(*Iterator);
                         });
    if (!IVinPhi) {
      LLVM_DEBUG(dbgs() << "Failed to make hw loop: ind var is not in phi\n");
      return llvm::None;
    }
  } else if (CmpUseDef->isPHI()) {
    // Likely a PreIncrement pattern
    Pattern = HWLoopPattern::PreIncrement;
    LLVM_DEBUG(dbgs() << "Process PreIncrement pattern\n");

    Phi = CmpUseDef;
    LLVM_DEBUG(dbgs() << "Phi node: "; Phi->dump());

    // Start phi instruction must be inside loop header.
    if (Phi->getParent() != Header) {
      LLVM_DEBUG(dbgs() << "Failed to make hw loop: "
                           "start phi node is out of the loop header\n");
      return llvm::None;
    }

    const llvm::Optional<LoopEIPhiInputs> StartPhiInputs =
        GetLoopEIPhiInputs(*Phi, *L);

    // Start phi must have input from the single Latch block
    if (!StartPhiInputs || StartPhiInputs->Internal.MBBOp->getMBB() != Latch) {
      LLVM_DEBUG(dbgs() << "Failed to make hw loop: "
                           "start phi input is not from the Latch BB\n");
      return llvm::None;
    }

    // Operand for <next_it> in Phi instruction
    const MachineOperand NextItOp =  *StartPhiInputs->Internal.ValOp;
    LLVM_DEBUG(dbgs() << "NextItOp: "; NextItOp.dump(););

    // <next_it> operand must be a register, otherwise hwloop optimization
    // is not applicable. Is it possible to be non-register at all?
    if (!NextItOp.isReg())
      return llvm::None;

    StepInstr = MRI->getVRegDef(NextItOp.getReg());
    LLVM_DEBUG(dbgs() << "StepInstr: "; StepInstr->dump());

    if (!L->contains(StepInstr) || !isIncrement(StepInstr->getOpcode())) {
      LLVM_DEBUG(dbgs() << "Failed to make hw loop: unsupported step instr\n");
      return llvm::None;
    }
  } else {
    LLVM_DEBUG(dbgs() << "Failed to make hw loop: unrecognized loop pattern\n");
    return llvm::None;
  }

  // TODO: Below I assume that step is the second operand and iterator is the first one.
  //       This is not necessary true.
  const MachineOperand &Step = StepInstr->getOperand(2);
  LLVM_DEBUG(dbgs() << "Step: "; Step.dump(););

  // Skip loop if step is defined inside the loop
  if (Step.isReg() && L->contains(MRI->getVRegDef(Step.getReg()))) {
    LLVM_DEBUG(dbgs() << "Failed to make hw loop: step is in the loop\n");
    return llvm::None;
  }

  const llvm::Optional<LoopEIPhiInputs> PhiInputs =
      GetLoopEIPhiInputs(*Phi, *L);
  if (!PhiInputs) {
    LLVM_DEBUG(dbgs() << "Failed to make hw loop: start phi has no edges "
                         "from blocks outside the loop\n");
    return llvm::None;
  }
  MachineOperand Start = *PhiInputs->External.ValOp;

  if (Start.isReg()) {
    MachineInstr* StartDef = MRI->getVRegDef(Start.getReg());
    if (StartDef->getOpcode() == TPC::MOVsip && !TII->isPredicated(*StartDef)) {
      // TODO: it would be good to remove the mov if it's not used.
      // But that leaves the vreg without any usage. LLVM crashes because of that.
      // I didn't find a way to delete a vreg yet. It looks like DCE and other
      // passes that delete instructions operate on the higher level IR,
      // not on this representation.
//      bool MultiUse = false;
//      for (MachineInstr& StartUser : MRI->use_nodbg_instructions(Start.getReg())) {
//        if (&StartUser != Phi) {
//          MultiUse = true;
//          break;
//        }
//      }

//      if (!MultiUse) {
//        StartDef->removeFromParent();
//      }
      Start = StartDef->getOperand(1);
    }
  }
  LLVM_DEBUG(dbgs() << "Start is "; Start.dump());

  // Skip loop if start is defined in the loop
  if (Start.isReg() && L->contains(MRI->getVRegDef(Start.getReg()))) {
    LLVM_DEBUG(dbgs() << "Failed to make hw loop: start is in the loop\n");
    return llvm::None;
  }

  LoopVars Lv;
  Lv.IsPredicated = false;

  Lv.Mode = getCmpMode(Condition->getOpcode());
  if (InvertCond)
    Lv.Mode = invertCond(Lv.Mode);
  if (NeedSwapCondition)
    Lv.Mode = swapCond(Lv.Mode);

  Lv.BoundarySel = Boundary->isImm();
  if (Lv.BoundarySel) {
    Lv.Boundary.ImmVal = Boundary->getImm();
  } else {
    Lv.Boundary.Reg = Boundary->getReg();
  }

  Lv.StepSel = Step.isImm();
  if (Lv.StepSel)
    Lv.Step.ImmVal = Step.getImm() + StepImmInc;
  else
    Lv.Step.Reg = Step.getReg();
  if (Lv.StepSel && !isIntN(32, Lv.Step.ImmVal)) {
    LLVM_DEBUG(dbgs() << "Failed to make hw loop: step is out of i32 type");
    return llvm::None;
  }

  Lv.StartSel = Start.isImm();
  if (Lv.StartSel)
    Lv.Start.ImmVal = Start.getImm();
  else
    Lv.Start.Reg = Start.getReg();

  // Try to get rid of != condition, it's to specific and prevents futher optimizations
  if (Lv.Mode == TPCII::LoopNE) {
    if (Lv.StepSel) {
      int64_t StepImm = Lv.Step.ImmVal;
      if (StepImm > 0) Lv.Mode = TPCII::LoopLT;
      else             Lv.Mode = TPCII::LoopGT;
    }
  }

  // Loops in PreIncrement might be trivially converted to hardware loops if
  // boundary is reassigned to boundary + step. But compiler must ensure that
  // boundary + step value will not overflow i32 type. This technique requires
  // boundary and step to be immediate values.
  //
  // TODO: LLVM might generate loops in PreIncrement pattern with
  //       non-immediate boundary. Such loops might also be converted to
  //       hardware but in a bit more complex way:
  //         * Ensure that start and step are immediate
  //         * Change iterator registry, start from start - step value
  //       Just ignore this option for simplicity now.
  if (Pattern == HWLoopPattern::PreIncrement) {
    if (!Lv.BoundarySel || !Lv.StepSel) {
      LLVM_DEBUG(dbgs() << "Failed to make hw loop: loop is in PreIncrement "
                           "form and boundary/step is not immediate\n");
      return llvm::None;
    }

    const int64_t FixedBoundaryImmVal = Lv.Boundary.ImmVal + Lv.Step.ImmVal;
    if (!isIntN(32, FixedBoundaryImmVal)) {
      LLVM_DEBUG(dbgs() << "Failed to make hw loop: loop is in PreIncrement "
                           "form and boundary + step value overflows i32 type\n");
      return llvm::None;
    }
    Lv.Boundary.ImmVal = FixedBoundaryImmVal;
  }

  LLVM_DEBUG(dbgs() << "Return LV\n");

  // Now that we're definetely going to convert this loop, remove all useless instructions
  auto CondUseMIs = MRI->use_instructions(Condition->getOperand(0).getReg());
  if (distance(CondUseMIs.begin(), CondUseMIs.end()) <= 1) {
    LLVM_DEBUG(dbgs() << "Remove: " << Condition);
    Condition->removeFromParent();
  }

  // Remove StepInstr if nobody inside loop uses its result except:
  //
  //   For HWLoopPattern::PostIncrement:
  //     start loop phi instruction (will be removed below)
  //     compare instruction against loop boundary (optionally removed above)
  //
  //   For HWLoopPattern::PreIncrement:
  //     start loop phi instruction (will be remove below)
  const Register IVReg = StepInstr->getOperand(0).getReg();
  const bool IteratorReuse =
      llvm::any_of(MRI->use_instructions(IVReg),
                   [L](MachineInstr &UseMI) {
                     return L->contains(&UseMI) && !UseMI.isPHI();
                   });
  if (!IteratorReuse) {
    LLVM_DEBUG(dbgs() << "Removing "; StepInstr->dump());
    StepInstr->removeFromParent();
  }

  Phi->removeFromParent();

  // Replace all occurrences of iterator with an SRF register
  LoopStats Stats(Phi->getOperand(0).getReg(), IVReg, L);
  if (Lv.StepSel) {
    Stats.ImmStep = true;
    Stats.Step.ImmVal = Lv.Step.ImmVal;
  } else {
    Stats.Step.Reg = Lv.Step.Reg;
  }

  LoopOrder.emplace(std::make_pair(L, Stats));

  return Lv;
}

void TPCHardwareLoops::setCounters(MachineLoop* L, int& Layer) {
  const auto LoopStatIt = LoopOrder.find(L);
  const bool IsOptimized = LoopStatIt != LoopOrder.end();

  if (IsOptimized) {
    ++Layer;
    LoopStatIt->second.CounterReg = TST->getHWLoopStartReg().id() + Layer;
    RegToLoop[LoopStatIt->second.CounterReg].push_back(LoopStatIt->second);
  }

  for (MachineLoop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    setCounters(*I, Layer);
  }

  if (IsOptimized) {
    --Layer;
  }
}

template <typename ProcessFunction>
void enumerateUsesToReplace(const Register Reg, const MachineRegisterInfo &MRI,
                            const ProcessFunction &Func) {
  for (auto I = MRI.reg_begin(Reg), E = MRI.reg_end(); I != E;) {
    MachineOperand &O = *I;
    ++I;
    Func(O);
  }
}

void TPCHardwareLoops::replaceIteratorsWithLoopCounter() {
  for (auto const &MapEntry : LoopOrder) {
    const LoopStats &Stats = MapEntry.second;

    MachineBasicBlock *const ExitMBB = Stats.L->getExitBlock();
    assert(ExitMBB);

    // Replace current iterator (phi) usages inside loop with loop counter.
    enumerateUsesToReplace(Stats.Phi, *MRI, [&](MachineOperand &MO) {
      if (Stats.L->contains(MO.getParent()))
        MO.substPhysReg(Stats.CounterReg, *MRI->getTargetRegisterInfo());
    });

    // Check current iterator value (phi) usages outside loop.
    if (!MRI->use_instructions(Stats.Phi).empty()) {
      // If current iterator value is not used in exit block phi, it means that
      // loop counter might be read in exit block. Otherwise, we need one more
      // instruction inside loop, which is less performant.
      const bool HasUsageInExitPhi =
          any_of(MRI->use_instructions(Stats.Phi), [&](const MachineInstr &MI) {
            return MI.isPHI() && MI.getParent() == ExitMBB;
          });
      if (HasUsageInExitPhi) {
        // Slower machine code:
        //   * Create COPY instruction inside loop:
        //     %curr_iterator_value = COPY %phys_loop_counter
        //   * Use %curr_iterator_value instead of Stats.Phi.
        MachineBasicBlock *const HeaderMBB = Stats.L->getHeader();
        const Register RegCopy = MRI->createVirtualRegister(&TPC::SRFRegClass);
        BuildMI(*HeaderMBB, HeaderMBB->getFirstNonPHI(), DebugLoc(),
                TII->get(TargetOpcode::COPY), RegCopy)
            .addReg(Stats.CounterReg);

        MRI->replaceRegWith(Stats.Phi, RegCopy);
      } else {
        // Faster machine code:
        //   * Create current value calculation instruction in exit block:
        //     %vir_reg_curr_iterator_value = SUB LoopCounter, LoopStep
        //   * Use %vir_reg_curr_iterator_value instead of Stats.Phi
        if (!ExitMBB->isLiveIn(Stats.CounterReg))
          ExitMBB->addLiveIn(Stats.CounterReg);
        const MachineOperand &StepOp = Stats.LoopMI->getOperand(2);
        const unsigned SubOpCode = StepOp.isImm() ? TPC::SUBsip : TPC::SUBssp;
        const Register RegCopy = MRI->createVirtualRegister(&TPC::SRFRegClass);
        BuildMI(*ExitMBB, ExitMBB->getFirstNonPHI(), DebugLoc(),
                TII->get(SubOpCode), RegCopy)
            .addReg(Stats.CounterReg)
            .add(StepOp)
            .addImm(TPCII::OpType::INT32)
            .addImm(0) // Switch
            .addReg(RegCopy, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);

        MRI->replaceRegWith(Stats.Phi, RegCopy);
      }
    }

    // If next iterator value (Iter) usages inside loop present, then next
    // iterator value definition instruction (StepInstr) is not deleted and
    // replacement for the next iterator value is not required. Otherwise,
    // replacement for the next iterator value is required iff it is used
    // outside loop.
    //   * Replace usages in exit MBB with loop counter.
    //   * If there are usages outside loop and exit MBB, copy loop counter to
    //     virtual register in exit MBB and replace usages with copy.
    if (!MRI->getVRegDef(Stats.Iter) &&
        !MRI->use_instructions(Stats.Iter).empty()) {
      if (!ExitMBB->isLiveIn(Stats.CounterReg))
        ExitMBB->addLiveIn(Stats.CounterReg);

      // Trick for copy creation on demand.
      Register CounterCopy;
      const auto GetVirtualCounterCopy = [&]() {
        if (!CounterCopy.isValid()) {
          CounterCopy = MRI->createVirtualRegister(&TPC::SRFRegClass);
          BuildMI(*ExitMBB, ExitMBB->getFirstTerminator(), DebugLoc(),
                  TII->get(TargetOpcode::COPY), CounterCopy)
              .addReg(Stats.CounterReg);
        }
        return CounterCopy;
      };

      enumerateUsesToReplace(Stats.Iter, *MRI, [&](MachineOperand &MO) {
        MachineInstr *const UseMI = MO.getParent();
        assert(!Stats.L->contains(UseMI));
        if (UseMI->getParent() == ExitMBB)
          MO.substPhysReg(Stats.CounterReg, *MRI->getTargetRegisterInfo());
        else
          MO.setReg(GetVirtualCounterCopy());
      });
    }
  }
}

template <typename MICheckFunction>
SmallVector<MachineOperand *, 4> collectUseOps(const Register Reg,
                                               const MICheckFunction &MIChecker,
                                               const MachineRegisterInfo &MRI) {
  SmallVector<MachineOperand *, 4> UseOps;
  for (MachineOperand &MO : MRI.use_operands(Reg)) {
    if (MIChecker(*MO.getParent()))
      UseOps.push_back(&MO);
  }
  return UseOps;
}

template <typename MICheckFunction>
void replaceRegUses(const Register SrcReg, const Register DstReg,
                    const MICheckFunction &MIChecker,
                    const MachineRegisterInfo &MRI) {
  for (MachineOperand *MO : collectUseOps(SrcReg, MIChecker, MRI))
    MO->setReg(DstReg);
}

/// Add an edge from loop predecessor to exit MBB if missing. Assume that
/// predecessor value is never used.
static void maybeAddEdgeFromPredecessorToExitMBB(MachineLoop &ML,
                                                 MachineRegisterInfo &MRI,
                                                 const TargetInstrInfo &TII) {
  MachineBasicBlock *const ExitMBB = ML.getExitBlock();
  MachineBasicBlock *const PredMBB = ML.getLoopPredecessor();
  MachineBasicBlock *const LatchMBB = ML.getLoopLatch();
  assert(ExitMBB && PredMBB && LatchMBB); // True by hw loop construction.

  if (ExitMBB->isPredecessor(PredMBB))
    return;

  const bool ExitMBBContainsPhis = ExitMBB->pred_size() > 1u;

  PredMBB->addSuccessor(ExitMBB);

  // Shortcut for fake register creation on demand.
  const auto MakePhiInputReg = [&](const Register LoopReg) {
    return insertImplicitDef(*MRI.getRegClass(LoopReg), *PredMBB,
                             std::prev(PredMBB->end()), MRI, TII);
  };

  if (ExitMBBContainsPhis) {
    // Easy way. Patch exit mbb phis by adding input from predecessor block.
    for (MachineInstr &ExitPhi : ExitMBB->phis()) {
      const Register LoopReg = getPhiValueForMBB(ExitPhi, *LatchMBB)->getReg();
      const Register Reg = MakePhiInputReg(LoopReg);
      ExitPhi.addOperand(MachineOperand::CreateReg(Reg, false));
      ExitPhi.addOperand(MachineOperand::CreateMBB(PredMBB));
    }
    return;
  }

  // Need to create phis for each loop register with usage outside loop.

  const auto InLoop = [&](const MachineInstr &MI) { return ML.contains(&MI); };

  // Collect registers which requires resolution.
  SmallVector<Register, 8> RegsToResolve;
  for (MachineBasicBlock *const LoopMBB : ML.getBlocks()) {
    for (const MachineInstr &MI : *LoopMBB) {
      for (const MachineOperand &MO : MI.defs()) {
        if (MO.getReg().isPhysical() ||
            all_of(MRI.use_instructions(MO.getReg()), InLoop))
          continue;
        RegsToResolve.push_back(MO.getReg());
      }
    }
  }

  // Actually, resolve.
  for (const Register LoopReg : RegsToResolve) {
    const Register FakeReg = MakePhiInputReg(LoopReg);

    const Register Reg = MRI.createVirtualRegister(MRI.getRegClass(LoopReg));
    BuildMI(*ExitMBB, ExitMBB->begin(), DebugLoc(), TII.get(TargetOpcode::PHI),
            Reg)
        .addReg(FakeReg)
        .addMBB(PredMBB)
        .addReg(LoopReg)
        .addMBB(LatchMBB);

    // Replace LoopReg usages outside loop with Reg.
    replaceRegUses(
        LoopReg, Reg,
        [&](const MachineInstr &MI) {
          return !InLoop(MI) && !(MI.isPHI() && MI.getParent() == ExitMBB);
        },
        MRI);
  }
}

void TPCHardwareLoops::addMissingEdges() {
  // Pass input graph might not contain edge from loop pre-header to exit block.
  // For example, if loop_taken pragma present or implicit loop_taken guarantee
  // exists. Such loop header does not contain reference to exit block, and
  // input CFG is correct.
  //
  // Pass adds LOOP instruction terminator into loop header with reference to
  // exit block. According to CFG restrictions, there must be an edge from loop
  // header to exit block even if it is never taken. At least overall LLVM code
  // implicitly assumes the following CFG invariant: "block with (terminator?)
  // instruction referred to another block must be a predecessor of that block".
  //
  // Let's add an edge if missing. Keep in mind that exit block phi input values
  // got from loop header are not meaningful, they are never used.
  for (auto const &Entry : LoopOrder)
    maybeAddEdgeFromPredecessorToExitMBB(*Entry.first, *MRI, *TII);
}

bool TPCHardwareLoops::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  if(!EnableTPCHardwareLoops)
    return false;
  LLVM_DEBUG(dbgs() << "********* TPC Hardware Loops *********\n");
  LLVM_DEBUG(MF.dump());
  bool Changed = false;

  MLI = &getAnalysis<MachineLoopInfo>();
  MRI = &MF.getRegInfo();
  MDT = &getAnalysis<MachineDominatorTree>();
  TST = &MF.getSubtarget<TPCSubtarget>();
  TII = TST->getInstrInfo();

  for (auto &L : *MLI) {
    if (!L->getParentLoop()) {
      int Layer = 0;
      Changed |= convertToHardwareLoop(L, Layer);
    }
  }

  // Fast return if no loop converted.
  if (!Changed)
    return false;

  // Now set all counter registers
  for (auto &L : *MLI) {
    if (!L->getParentLoop()) {
      int Layer = -1;
      setCounters(L, Layer);
    }
  }

  for (auto const& MapEntry: LoopOrder) {
    const LoopStats &Stats = MapEntry.second;
    Stats.LoopMI->addOperand(MF, MachineOperand::CreateReg(Stats.CounterReg, true, true));
    Stats.LoopendMI->addOperand(MF, MachineOperand::CreateReg(Stats.CounterReg, true, true));
  }

  replaceIteratorsWithLoopCounter();

  // Fix any phi-node and tied registers that we broke
  for (MachineFunction::iterator I = MF.begin(), E = MF.end(); I != E; ++I) {
    MachineBasicBlock* Block = &*I;
    for (MachineInstr& MI : Block->instrs()) {

      if (MI.isPHI()) {
        const SmallVector<PhiIP, 4> PhiInputs = getAllPhiIPs(MI);
        for (const PhiIP IP : PhiInputs) {
          if (!IP.ValOp->isReg())
            continue;

          const Register ValReg = IP.ValOp->getReg();

          if (!ValReg.isPhysical())
            continue;

          // Do not touch phys registers which are not newly introduced loop
          // counters.
          auto LoopsIt = RegToLoop.find(ValReg);
          if (LoopsIt == RegToLoop.end())
            continue;

          MachineBasicBlock* const Origin = IP.MBBOp->getMBB();

          MachineBasicBlock::iterator InsertPos = Origin->end();
          MachineBasicBlock::iterator LastInst = --(Origin->end());
          while(InsertPos != Origin->begin() && (*LastInst).isTerminator()) {
            --InsertPos;
            --LastInst;
          }

          const auto &LSVector = LoopsIt->second;
          const auto MatchedLoop =
              llvm::find_if(LSVector, [=](const LoopStats &LS) {
                return LS.L->contains(Origin);
              });
          assert(MatchedLoop != LSVector.end());

          Register SafeReg = MRI->createVirtualRegister(&TPC::SRFRegClass);
          if (!MatchedLoop->L->contains(&MI)) {
            if (MatchedLoop->ImmStep) {
              BuildMI(*Origin, InsertPos, DebugLoc(), TII->get(TPC::ADDsip), SafeReg)
                  .addReg(ValReg)
                  .addImm(MatchedLoop->Step.ImmVal)
                  .addImm(TPCII::OpType::INT32)
                  .addImm(0)
                  .addReg(SafeReg, RegState::Undef)
                  .addReg(TPC::SPRF_TRUE)
                  .addImm(0);
            } else {
              BuildMI(*Origin, InsertPos, DebugLoc(), TII->get(TPC::ADDssp), SafeReg)
                  .addReg(ValReg)
                  .addReg(MatchedLoop->Step.Reg)
                  .addImm(TPCII::OpType::INT32)
                  .addImm(0)
                  .addReg(SafeReg, RegState::Undef)
                  .addReg(TPC::SPRF_TRUE)
                  .addImm(0);
            }
          } else {
            BuildMI(*Origin, InsertPos, DebugLoc(),
                    TII->get(TargetOpcode::COPY), SafeReg)
                .addReg(ValReg);
          }
          IP.ValOp->setReg(SafeReg);
        }
        continue;
      }

      // Not a phi.
      for (MachineOperand &MO : MI.uses()) {
        if (!MO.isReg() || !MO.isTied())
          continue;

        const Register Reg = MO.getReg();

        if (!Reg.isPhysical())
          continue;

        // Do not touch phys registers which are not newly introduced loop
        // counters.
        if (RegToLoop.count(Reg.id()) == 0)
          continue;

        Register SafeReg = MRI->createVirtualRegister(&TPC::SRFRegClass);
        BuildMI(*MI.getParent(), MI, DebugLoc(),
                TII->get(TargetOpcode::COPY), SafeReg)
            .addReg(MO.getReg());
        MO.setReg(SafeReg);
        LLVM_DEBUG(dbgs() << "Doing something with tied operands: " << MI);
      }
    }
  }

  addMissingEdges();

  LLVM_DEBUG(if (Changed) {
    dbgs() << "After conversion:\n";
    MF.dump();
  });

  for (MachineBasicBlock *MBB : RemoveList) {
    MLI->removeBlock(MBB);
    MBB->eraseFromParent();
  }

  return Changed;
}

bool TPCHardwareLoops::convertToHardwareLoop(MachineLoop *L,
                                             int &Layer) {
  LLVM_DEBUG(dbgs() << "Convert loop\n");
  LLVM_DEBUG(L->dump());

  // This is just for sanity.
  assert(L->getHeader() && "Loop without a header?");

  bool Changed = false;

  // Process nested loops first.
  int MaxLayer = Layer;
  for (MachineLoop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    int TempLayer = Layer;
    Changed |= convertToHardwareLoop(*I, TempLayer);
    if (TempLayer > MaxLayer) {
      MaxLayer = TempLayer;
    }
  }
  Layer = MaxLayer;

  // Only 4 innermost loops can be transformed.
  if (Layer >= 4)
    return Changed;

  if (!isHWLoopTransformationAllowed(*L, *MLI, *TST))
    return Changed;

  llvm::Optional<LoopVars> Lv = calculateLoop(L);
  if (!Lv)
    return Changed;

  MachineBasicBlock *Header = L->getHeader();
  MachineBasicBlock *Latch = L->getLoopLatch();

  MDNode* LoopMD = Latch->getBasicBlock()->getTerminator()->getMetadata(LLVMContext::MD_loop);
  Lv->IsTaken = LoopMD && hasLoopTakenMD(*LoopMD);

  LLVM_DEBUG(Lv->dump());

  typedef MachineBasicBlock::reverse_instr_iterator rev_interator;
  rev_interator I = Latch->instr_rbegin();
  MachineInstr *Terminator = &*I;

  // Remove terminators
  if (Terminator->getOpcode() == TPC::JMPR_u) {
    Latch->remove(Terminator);

    I = Latch->instr_rbegin();
    Terminator = &*I;

    assert((Terminator->getOpcode() == TPC::JMPR) &&
           "Regular loop should end with a conditional branch");

    Latch->remove(Terminator);
  } else {
    assert((Terminator->getOpcode() == TPC::JMPR) &&
           "Regular loop should end with a conditional branch");

    Latch->remove(Terminator);
  }

  MachineBasicBlock::iterator InsertPos;
  MachineBasicBlock* LoopBlock = nullptr;

  MachineBasicBlock* Preheader = nullptr;
  typedef std::vector<MachineBasicBlock*> MBBVector;
  MBBVector Preds(Header->pred_begin(), Header->pred_end());

  for (MachineBasicBlock *PB : Preds) {
    if (PB != Latch) {
      Preheader = PB;
    }
  }
  assert(Preheader && "Unreachable loop");
 
  MachineBasicBlock* NewPreheader = stripEntranceCheck(Preheader, Header, L->getExitBlock(), Lv.getPointer());
  if (NewPreheader) {
    InsertPos = NewPreheader->end();
    LoopBlock = NewPreheader;
  } else if (Preheader->succ_size() == 1) {
    InsertPos = Preheader->end();
    LoopBlock = Preheader;
  } else {
    MachineFunction* MF = Header->getParent();
    MachineBasicBlock* Prolog = MF->CreateMachineBasicBlock();
    MF->insert(Header->getIterator(), Prolog);

    // Insert loop pre-header for the prolog and rewire CFG.
    for (MachineBasicBlock *PB : Preds) {
      if (PB != Latch) {
        //Patch Phis
        for (MachineInstr& MI : Header->instrs()) {
          if (MI.isPHI()) {
            for (MachineOperand& MO: MI.uses()) {
              if (MO.isMBB() && MO.getMBB() == PB) {
                MO.setMBB(Prolog);
              }
            }
          }
        }

        PB->ReplaceUsesOfBlockWith(Header, Prolog);
      }
    }

    Prolog->addSuccessor(Header, BranchProbability::getOne());
    MachineLoop *ParentLoop = L->getParentLoop();
    if (ParentLoop && !MLI->getLoopFor(Prolog)) {
      ParentLoop->addBasicBlockToLoop(Prolog, MLI->getBase());
    }

    InsertPos = Prolog->end();
    LoopBlock = Prolog;
  }

  //DebugLoc DL = Prolog->begin()->getDebugLoc();

  unsigned LOOP_opc = chooseLoopOpcode(Lv.getValue());
  const MachineInstrBuilder& MIB = BuildMI(*LoopBlock, InsertPos, DebugLoc(), TII->get(LOOP_opc));

  if (Lv->StartSel) {
    MIB.addImm(Lv->Start.ImmVal);
  } else {
    MIB.addReg(Lv->Start.Reg);
  }

  if (Lv->BoundarySel) {
    MIB.addImm(Lv->Boundary.ImmVal);
  } else {
    MIB.addReg(Lv->Boundary.Reg);
  }

  if (Lv->StepSel) {
    MIB.addImm(Lv->Step.ImmVal);
  } else {
    MIB.addReg(Lv->Step.Reg);
  }

  MIB.addImm(Lv->Mode);
  MachineBasicBlock* Block = L->getExitBlock();
  assert(Block);
  MIB.addMBB(Block);

  if (Lv->IsPredicated) {
    MIB.addReg(Lv->PredReg);
    MIB.addImm(Lv->Polarity);
  }
  const auto LoopStatIt = LoopOrder.find(L);
  LoopStatIt->second.LoopMI = MIB.getInstr();
  InsertPos = Latch->end();
 //DL = InsertPos->getDebugLoc();
  const MachineInstrBuilder& MEndLoop = BuildMI(*Latch, InsertPos, DebugLoc(), TII->get(TPC::LOOPEND));
  MEndLoop.addMBB(Block);
  MEndLoop.addMBB(Header);
// Below issue is done for G-1819
// it is temporaly hidden due to new exposed problem G-1926
 #if 0 
  // need for unhardware to add upper,step
  if (!Lv->BoundarySel) {
    MEndLoop.addReg(Lv->Boundary.Reg, RegState::Implicit);
  }
  
  if (!Lv->StepSel) {
    MEndLoop.addReg(Lv->Step.Reg, RegState::Implicit);
  } 
#endif
  if (LoopMD) {
    MEndLoop.addMetadata(LoopMD);
  }

  LoopStatIt->second.LoopendMI = MEndLoop.getInstr();
 
  ++Layer;
  return true;
}

bool TPCHardwareLoops::predicateLoop(MachineBasicBlock* Preheader, MachineBasicBlock* Exit, MachineBasicBlock* PredTarget, LoopVars* Lv) {
  MachineInstr *Terminator = &(*(--Preheader->end()));

  if (Terminator->getOpcode() == TPC::JMPR) {
    if (Terminator->getOperand(0).getMBB() != Exit) {
      return false;
    }
    Lv->PredReg = Terminator->getOperand(1).getReg();
    Lv->Polarity = !Terminator->getOperand(2).getImm();
    Lv->IsPredicated = true;
  }
  // There's a preheader block that's not part of the loop, but under jump guard, make every instruction there predicated
//  if (PredTarget != nullptr) {
//    for (MachineInstr& MI : PredTarget->instrs()) {
//      unsigned PredPos = MI.getNumOperands() - 2; // All instructions except pseudos have predicate at the penultimate position
//      if(MI.getNumOperands() > 3) { // pred_link, pred, polarity
//        MI.getOperand(PredPos).setReg(Terminator->getOperand(1).getReg());
//      }
//    }
//  }
  Terminator->removeFromParent();
  return true;
}

static void squashFullCopies(MachineBasicBlock &MBB,
                             const MachineRegisterInfo &MRI) {
  using MIContainer = SmallVector<MachineInstr *, 4>;

  DenseMap<Register, MIContainer> SrcRegToCopyMIs;
  for (MachineInstr &MI : MBB) {
    if (MI.isFullCopy())
      SrcRegToCopyMIs[MI.getOperand(1).getReg()].push_back(&MI);
  }

  for (const auto &Item : SrcRegToCopyMIs) {
    const MIContainer &MIs = Item.second;
    for (unsigned I = 1, E = MIs.size(); I < E; ++I) {
      replaceRegUses(
          MIs[I]->getOperand(0).getReg(), MIs[0]->getOperand(0).getReg(),
          [](const auto &) { return true; }, MRI);
      MIs[I]->removeFromParent();
    }
  }
}

// Attempts to remove conditions and jumps that guard a post-condition loop
MachineBasicBlock* TPCHardwareLoops::stripEntranceCheck(MachineBasicBlock* Preheader, MachineBasicBlock* Header, MachineBasicBlock* Exit, LoopVars* Lv) {
  bool ShouldMerge = false;
  bool Predicated  = false;

  MachineBasicBlock* LoopEntrance = Header;
  MachineBasicBlock* PredTarget = nullptr;
  if (Preheader->empty()) {
    // TODO: guarding condition is in another block. We should attempt to find it anyway.
    //       This single if might not be enough
    if (Preheader->pred_size() == 1) {
      LoopEntrance = Preheader;
      Preheader = *(Preheader->predecessors().begin());
      ShouldMerge = true;
    } else {
      return nullptr;
    }
  }

  MachineInstr *Terminator = &(*(--Preheader->end()));
  // Guarding jump may be in predecessor of preheader if preheader has some code hoisted to it
  if (Terminator->getOpcode() != TPC::JMPR) {
    // We remove jumps around loop only if there's loop_taken pragma meaning that
    // the loop is executed at least once. We can do the optimization for any loop
    // but that requires altering instructions:
    // In one scenario guarding jump falls through right into the loop body and LOOP
    // instruction will perform the check. In the other scenario there's some
    // preparation code bedore loop body that is not a part of the loop and guarding
    // jump falls through to that. If we remove jump and the loop is never executed
    // we also should not execute that prolog. In order to do this we have to
    // predicate everything in the prolog. There're two problems with that:
    //   1. Load and Store slot are currently can't share predicates and will not
    //      be bundled together
    //   2. We can't predicate pseudo instructions and delay-predicate phi nodes.
    //      This problem causes some tests to fail because when later phi-nodes
    //      are eliminated resulted MOVs should be predicated but LLVM can't do that.
    //      As a result some values are overwritten.
    if (!Lv->IsTaken) {
      return nullptr;
    }

    // Preheader must be just a connector block to a loop body
    if (Preheader->succ_size() == 1 && Preheader->pred_size() == 1) {
      LoopEntrance = Preheader;
      PredTarget = Preheader;
      Preheader = *(Preheader->predecessors().begin());
      Terminator = &(*(--Preheader->end()));
      ShouldMerge = true;
    }
  }

  // TODO: there are some weird layouts where preheader jumps to the successor of the exit
  if (!Preheader->isSuccessor(Exit)) {
    return nullptr;
  }

  if (Terminator->getOpcode() == TPC::JMPR) {
    MachineInstr* Condition = MRI->getVRegDef(Terminator->getOperand(1).getReg());
    if (Condition->isCopy() || Condition->getOpcode() == TPC::MOVpsp) {
      // Try looking further
      // TODO: What if it's still not a cmp
      Condition = MRI->getVRegDef(Condition->getOperand(1).getReg());
    }

    //LLVM_DEBUG(dbgs() << "Condition: "; Condition->dump());
    LLVM_DEBUG(dbgs() << "Terminator: "; Terminator->dump());
    LLVM_DEBUG(Preheader->getParent()->dump());

    MachineOperand& Op1 = Condition->getOperand(1);
    MachineOperand& Op2 = Condition->getOperand(2);

    // Check that preheader actually jumps to the exit
    if (size(Preheader->successors()) != 2 ||
        !is_contained(Preheader->successors(), LoopEntrance) ||
        !is_contained(Preheader->successors(), Exit))
      return nullptr;
    LLVM_DEBUG(dbgs() << "Checked CFG\n");

    // Check that cmp instruction involves both start and boundary of the loop
    if (!RemoveJumps) {
      LLVM_DEBUG(dbgs() << "Op1 "; Op1.dump());
      LLVM_DEBUG(dbgs() << "Op2 "; Op2.dump());
      if (Lv->isStart(Op1)) {
        if (!Lv->isBoundary(Op2)) {
          if (predicateLoop(Preheader, Exit,  PredTarget, Lv)) {
            Predicated = true;
          } else {
            return nullptr;
          }
        }
      } else if (Lv->isBoundary(Op1)) {
        if (!Lv->isStart(Op2)) {
          if (predicateLoop(Preheader, Exit,  PredTarget, Lv)) {
            Predicated = true;
          } else {
            return nullptr;
          }
        }
      } else {
          if (predicateLoop(Preheader, Exit,  PredTarget, Lv)) {
            Predicated = true;
          } else {
            return nullptr;
          }
      }
    }
  } else {
    return nullptr;
  }

  if (!Predicated) {
    LLVM_DEBUG(dbgs() << "Checked operands\n");
    // If we are here, we can safely eliminate jump and merge blocks
    MachineInstr* Cond = MRI->getVRegDef(Terminator->getOperand(1).getReg());
    const auto CondUseMIs = MRI->use_instructions(Cond->getOperand(0).getReg());
    if (distance(CondUseMIs.begin(), CondUseMIs.end()) <= 1) {
      LLVM_DEBUG(dbgs() << "Remove: " << Cond);
      Cond->removeFromParent();
    }
    LLVM_DEBUG(dbgs() << "Remove: " << Terminator);
    Terminator->removeFromParent();
  }

  if (ShouldMerge) {
    assert((LoopEntrance->empty() || !(*LoopEntrance->begin()).isPHI()) && "Connector block can't have phis");
    MachineBasicBlock* LoopBody = *LoopEntrance->succ_begin();
    Preheader->splice(Preheader->end(), LoopEntrance, LoopEntrance->begin(), LoopEntrance->end());
    Preheader->removeSuccessor(LoopEntrance);
    Preheader->addSuccessor(*LoopEntrance->succ_begin());
    LoopEntrance->removeSuccessor(LoopEntrance->succ_begin());

    for (MachineInstr& MI : LoopBody->phis()) {
      for (MachineOperand& MO : MI.uses()) {
        if (MO.isMBB() && MO.getMBB() == LoopEntrance) {
          MO.setMBB(Preheader);
        }
      }
    }

    RemoveList.push_back(LoopEntrance);

    if (Lv->IsTaken) {
      Preheader->removeSuccessor(Exit);

      // Fix phis because loop is always taken, can't get to exit from Preheader
      const auto InsertPos = Exit->getFirstNonPHI();

      SmallVector<MachineInstr *, 4> Removal;
      for (MachineInstr &MI : Exit->phis()) {
        // We can add phi-nodes inside this loop. Need to check whether this is
        // new or old phi. This is bad, but I don't know a better solution.
        const bool Processed =
            all_of(MI.uses(), [Preheader](MachineOperand &Op) {
              return !Op.isMBB() || Op.getMBB() != Preheader;
            });
        if (Processed)
          continue;

        // 2-way merge, replace phi with copy
        if (MI.getNumOperands() == 5) {
          unsigned UseReg = 0;
          unsigned DefReg = MI.getOperand(0).getReg();
          if (MI.getOperand(2).getMBB() == Preheader) {
            UseReg = MI.getOperand(3).getReg();
          } else {
            UseReg = MI.getOperand(1).getReg();
          }
          Removal.push_back(&MI);
          BuildMI(*Exit, InsertPos, DebugLoc(), TII->get(TargetOpcode::COPY),
                  DefReg)
              .addReg(UseReg);
        } else { // Exit has some other predecessors, remove Preheader case from
                 // phi
          unsigned DefReg = MI.getOperand(0).getReg();
          for (unsigned i = 1; i < MI.getNumOperands(); ++i) {
            if (MI.getOperand(i).isMBB() &&
                MI.getOperand(i).getMBB() == Preheader) {
              MachineInstrBuilder BMI =
                  BuildMI(*Exit, InsertPos, DebugLoc(),
                          TII->get(TargetOpcode::PHI), DefReg);
              for (unsigned j = 1; j < MI.getNumOperands(); ++j) {
                if (j == i - 1) {
                  ++j;
                  continue;
                }
                BMI.addReg(MI.getOperand(j++).getReg());
                BMI.addMBB(MI.getOperand(j).getMBB());
              }
              Removal.push_back(&MI);
              break;
            }
          }
        }
      }

      for (MachineInstr *MI : Removal) {
        MI->removeFromParent();
      }

      squashFullCopies(*Exit, *MRI);
    }
  }

  return Preheader;
}

char TPCHardwareLoops::ID = 0;

TPCII::CmpMode TPCHardwareLoops::getCmpMode(unsigned Opcode) const {
  switch (Opcode) {
    case TPC::CMP_EQssp:
    case TPC::CMP_EQsip:
      return TPCII::LoopEQ;
    case TPC::CMP_NEQssp:
    case TPC::CMP_NEQsip:
      return TPCII::LoopNE;
    case TPC::CMP_LESSssp:
    case TPC::CMP_LESSsip:
      return TPCII::LoopLT;
    case TPC::CMP_LEQssp:
    case TPC::CMP_LEQsip:
      return TPCII::LoopLE;
    case TPC::CMP_GRTssp:
    case TPC::CMP_GRTsip:
      return TPCII::LoopGT;
    case TPC::CMP_GEQssp:
    case TPC::CMP_GEQsip:
      return TPCII::LoopGE;
    default:
      return TPCII::LoopErr;
  }
}

TPCII::CmpMode TPCHardwareLoops::invertCond(TPCII::CmpMode mode) const {
  switch(mode) {
    case TPCII::LoopEQ: return TPCII::LoopNE;
    case TPCII::LoopNE: return TPCII::LoopEQ;
    case TPCII::LoopLT: return TPCII::LoopGE;
    case TPCII::LoopLE: return TPCII::LoopGT;
    case TPCII::LoopGT: return TPCII::LoopLE;
    case TPCII::LoopGE: return TPCII::LoopLT;
    default: llvm_unreachable("Incorrect cmp mode");
  }
}

TPCII::CmpMode TPCHardwareLoops::swapCond(TPCII::CmpMode mode) const {
  switch(mode) {
    case TPCII::LoopEQ: return TPCII::LoopEQ;
    case TPCII::LoopNE: return TPCII::LoopNE;
    case TPCII::LoopLT: return TPCII::LoopGT;
    case TPCII::LoopLE: return TPCII::LoopGE;
    case TPCII::LoopGT: return TPCII::LoopLT;
    case TPCII::LoopGE: return TPCII::LoopLE;
    default: llvm_unreachable("Incorrect cmp mode");
  }
}

unsigned TPCHardwareLoops::chooseLoopOpcode(const LoopVars Lv) const {
  return getLoopMachineInstrOpcode(Lv.StartSel, Lv.BoundarySel, Lv.StepSel,
                                   Lv.IsPredicated);
}

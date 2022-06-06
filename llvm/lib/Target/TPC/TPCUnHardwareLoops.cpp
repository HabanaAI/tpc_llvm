//===- TPCUnHardwareLoop.cpp ------------------===//
//
//===-Transform HW Loop in usual loop------------------------------------===//
//
//===-if it exceeds limit ----------------------------------------------===//

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "TPCVLIWPacketizer.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/Support/Debug.h"

#include <bitset>

using namespace llvm;

#define DEBUG_TYPE "tpc-unhardware-loops"

namespace llvm {
FunctionPass *createTPCUnHardwareLoops();
void initializeTPCUnHardwareLoopsPass(PassRegistry &);
} // namespace llvm

static const char PassDescription[] = "TPC UnHardware Loops";
static const char PassName[] = "tpc-unhardware-loops";

// Flag to disable register balancing.
static cl::opt<bool> EnableTPCUnHardwareLoops(
    PassName, cl::desc("transform hardware loops in usual loops"),
    cl::init(true), cl::Hidden);

static cl::opt<bool> // debugging purposes
    ForceUnHardwareLoops(
        "tpc-force-unhardware-loops",
        cl::desc("transform hardware lops in usual loops of any size"),
        cl::init(false), cl::Hidden);

static cl::opt<unsigned> UnHardwareInstructionsMinCount(
    "tpc-unhardware-loops-limit", cl::init(1950), cl::Hidden,
    cl::Optional,
    cl::desc("Minimum size of machine loop in instructions to be converted "
             "from hardware form to software form"));

#define EMPTYREF (unsigned)(~0)

static unsigned getLoopRefOrDef(const unsigned Ref, const unsigned Default) {
  assert(Default != EMPTYREF);
  return Ref != EMPTYREF ? Ref : Default;
}

namespace {
class TPCUnHardwareLoops : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  const TPCSubtarget *TST = nullptr;
  const TargetInstrInfo *TII = nullptr;

public:
  static char ID;
  TPCUnHardwareLoops() : MachineFunctionPass(ID) {
    initializeTPCUnHardwareLoopsPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<MachineBranchProbabilityInfo>();
    AU.addRequired<MachineLoopInfo>();
    AU.addPreserved<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  StringRef getPassName() const override { return PassDescription; }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  struct TLoopFrame {
    MachineInstr *StartInstr = nullptr;
    MachineInstr *EndInstr = nullptr;
    Register LoopCounter;
    Register NewLoopCounter;
    unsigned InstrNumber = 0;
    unsigned SPRegNum = 0;
    // Tree references
    unsigned UpLoop = EMPTYREF;
    unsigned RightLoop = EMPTYREF;
    unsigned DownLoop = EMPTYREF;
    unsigned LastClosedSon = EMPTYREF;
    SmallSet<MachineInstr *, 16> LoopRegInstr;
    bool Spoiled = false;
    bool Transform = false;
  };

  unsigned CurrentTopLoop = EMPTYREF;
  std::bitset<16> OccupiedSPRegNumbers; // SP0 .. SP15
  SmallVector<TLoopFrame, 64> HWLoops;

  void transformHWLoopBack(unsigned L);
  void checkDefUsedInLoopTunes(const MachineOperand &MO, unsigned L);
  void processNotLoopInstr(MachineInstr &MI);
  bool isLoopTreeSpoiled(unsigned RootL);
  void markTreeAsSpoiled(unsigned RootL);
  void escalateSpoiled();
  void markTreeToTransformUp(unsigned L);
  void processStartLoop(MachineInstr &MI, unsigned MICount);
  unsigned processEndLoop(MachineInstr &MI, unsigned MICount);
  void setRight(unsigned EndLoop);
  unsigned findElderBrother(unsigned EndLoop);
  unsigned reachRight(unsigned LeftMost);
  bool isTopMostHWLoop(unsigned L) const;
  void setNewCounts();
  void setHWCount(unsigned RootL, Register Counter);
  void setSoftCounts(unsigned RootFrom, Register Counter);

  /// Parse function and construct HWLoops tree data.
  void constructHWLoopsTree();

  /// Mark hw loops that to transform to sw form.
  void markLoopsToUnHardware();

  /// Check if there is a hw loop marked for transformation to sw form.
  bool hasLoopToUnHardware();

  /// Transform marked hw loops to sw form.
  void unHardwareLoops();

  struct CountersReplacementCtx {
    SmallSet<MachineInstr *, 16> UpdatedMIs;
    SmallVector<Register, 4> TransReg;
  };

  /// Recursively replace loops counters usage after transformation.
  void replaceCounts(CountersReplacementCtx &Ctx, unsigned RootL = 0);

  /// Debug dump hw loops tree
  void dumpHWLoops();
  void dumpHWLoop(unsigned L, unsigned Indent);
};
} // namespace

char TPCUnHardwareLoops::ID = 0;

INITIALIZE_PASS(TPCUnHardwareLoops, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCUnHardwareLoops() {
  return new TPCUnHardwareLoops();
}

// TODO: SP0 is incorrectly supported here.
static int getSPNumber(Register SPReg) {
  switch (SPReg) {
  case TPC::SPRF_TRUE:
    return 0;
  case TPC::SP1:
  case TPC::SP2:
  case TPC::SP3:
  case TPC::SP4:
  case TPC::SP5:
  case TPC::SP6:
  case TPC::SP7:
  case TPC::SP8:
  case TPC::SP9:
  case TPC::SP10:
  case TPC::SP11:
  case TPC::SP12:
  case TPC::SP13:
  case TPC::SP14:
  case TPC::SP15:
    return SPReg - TPC::SP0;
  }
  return -1;
}

// TODO: SP0 is incorrectly supported here.
static Register getSPRegister(const int RegNum) {
  assert(0 <= RegNum && RegNum < 16);
  if (RegNum == 0)
    return TPC::SPRF_TRUE;
  return TPC::SP0 + RegNum;
}

static unsigned getCMPCode(const MachineOperand &LoopUpperBound,
                           const unsigned CmpMode) {
  const bool IsImm = LoopUpperBound.isImm();
  switch (CmpMode) {
    // clang-format off
  case TPCII::LoopEQ: return IsImm ? TPC::CMP_EQsip   : TPC::CMP_EQssp;
  case TPCII::LoopNE: return IsImm ? TPC::CMP_NEQsip  : TPC::CMP_NEQssp;
  case TPCII::LoopLT: return IsImm ? TPC::CMP_LESSsip : TPC::CMP_LESSssp;
  case TPCII::LoopLE: return IsImm ? TPC::CMP_LEQsip  : TPC::CMP_LEQssp;
  case TPCII::LoopGT: return IsImm ? TPC::CMP_GRTsip  : TPC::CMP_GRTssp;
  case TPCII::LoopGE: return IsImm ? TPC::CMP_GEQsip  : TPC::CMP_GEQssp;
    // clang-format on
  }
  llvm_unreachable("Incorrect cmp mode");
}

static bool isLoopInstr(const MachineInstr &MI) {
  switch (MI.getOpcode()) {
  case TPC::LOOP1iiip:
  case TPC::LOOP1iisp:
  case TPC::LOOP1isip:
  case TPC::LOOP1issp:
  case TPC::LOOP1siip:
  case TPC::LOOP1sisp:
  case TPC::LOOP1ssip:
  case TPC::LOOP1sssp:
  case TPC::LOOPiii:
  case TPC::LOOPiiip:
  case TPC::LOOPiis:
  case TPC::LOOPiisp:
  case TPC::LOOPisi:
  case TPC::LOOPisip:
  case TPC::LOOPiss:
  case TPC::LOOPissp:
  case TPC::LOOPsii:
  case TPC::LOOPsiip:
  case TPC::LOOPsis:
  case TPC::LOOPsisp:
  case TPC::LOOPssi:
  case TPC::LOOPssip:
  case TPC::LOOPsss:
  case TPC::LOOPsssp:
    return true;
  }
  return false;
}

static bool isPrLoopInstr(const MachineInstr *MI) {
  switch (MI->getOpcode()) {
  case TPC::LOOP1iiip:
  case TPC::LOOP1iisp:
  case TPC::LOOP1isip:
  case TPC::LOOP1issp:
  case TPC::LOOP1siip:
  case TPC::LOOP1sisp:
  case TPC::LOOP1ssip:
  case TPC::LOOP1sssp:
  case TPC::LOOPiiip:
  case TPC::LOOPiisp:
  case TPC::LOOPisip:
  case TPC::LOOPissp:
  case TPC::LOOPsiip:
  case TPC::LOOPsisp:
  case TPC::LOOPssip:
  case TPC::LOOPsssp:
    return true;
  }
  return false;
}

static bool isLoopRegUse(const MachineOperand &MO, const TPCSubtarget &ST) {
  return MO.isReg() && MO.isUse() && ST.isHWLoopReg(MO.getReg());
}

static bool isWithLoopReg(const MachineInstr &MI, const TPCSubtarget &ST) {
  if (MI.getOpcode() == TPC::LOOPEND)
    return false;

  return std::any_of(
      MI.operands_begin(),
      MI.operands_begin() + (isLoopInstr(MI) ? 3 : MI.getNumOperands()),
      [&](const MachineOperand &MO) { return isLoopRegUse(MO, ST); });
}

void TPCUnHardwareLoops::transformHWLoopBack(unsigned L) {
  const TLoopFrame &LF = HWLoops[L];
  MachineInstr *const StartInstr = LF.StartInstr;
  MachineInstr *const EndInstr = LF.EndInstr;
  const Register LoopCount = LF.LoopCounter;

  MachineBasicBlock *const BBLoop = EndInstr->getParent();
  MachineBasicBlock *const AfterLoop = BBLoop->getNextNode();
  MachineBasicBlock *const BBPre = StartInstr->getParent();
  MachineBasicBlock *const Loop1stBB = BBPre->getNextNode();
  const MachineOperand Init = StartInstr->getOperand(0);
  const MachineOperand UpB = StartInstr->getOperand(1);
  const MachineOperand Step = StartInstr->getOperand(2);
  unsigned CmpK = StartInstr->getOperand(3).getImm();
  MachineInstrBuilder MIB;
  const MachineBasicBlock::iterator NMIs = BBPre->instr_end();

  AfterLoop->setLabelMustBeEmitted();
  BBLoop->setLabelMustBeEmitted();
  if (isPrLoopInstr(StartInstr)) {
    MachineOperand LoopPred = StartInstr->getOperand(5);
    MachineOperand LoopPolar = StartInstr->getOperand(6);
    bool Polar = LoopPolar.getImm();
    // Jump due to loop predicate
    MIB = BuildMI(*BBPre, NMIs, StartInstr->getDebugLoc(), TII->get(TPC::JMPR));
    MIB.addMBB(AfterLoop);
    MIB.add(LoopPred);
    MIB.addImm(!Polar);
  }

  const MCInstrDesc &DeskMOVinit =
      TII->get(Init.isImm() ? TPC::MOVsip : TPC::MOVssp);
  MIB =
      BuildMI(*BBPre, NMIs, StartInstr->getDebugLoc(), DeskMOVinit, LoopCount);
  MIB.add(Init);
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(LoopCount, RegState::Undef);
  MIB.addReg(TPC::SPRF_TRUE);
  MIB.addImm(0);

  HWLoops[L].LoopRegInstr.insert(MIB.getInstr());

  // CMP now
  Register RegCmp = getSPRegister(LF.SPRegNum);
  assert(RegCmp);

  unsigned CmpCode = getCMPCode(UpB, CmpK);
  MIB = BuildMI(*BBPre, NMIs, StartInstr->getDebugLoc(), TII->get(CmpCode),
                RegCmp);
  MIB.addReg(LoopCount);
  MachineOperand HereUpB = UpB;
  if (HereUpB.isReg()) {
    HereUpB.setIsKill(false);
  }
  MIB.add(HereUpB);
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(RegCmp, RegState::Undef);
  MIB.addReg(TPC::SPRF_TRUE);
  MIB.addImm(0);

  HWLoops[L].LoopRegInstr.insert(MIB.getInstr());

  // JMP now
  MIB = BuildMI(*BBPre, NMIs, StartInstr->getDebugLoc(), TII->get(TPC::JMPR));
  MIB.addMBB(AfterLoop);
  MIB.addReg(RegCmp, RegState::Kill);
  MIB.addImm(1); // Polarity.

  RegCmp = getSPRegister(LF.SPRegNum);
  assert(RegCmp);

  // need to take right upper and Step
  unsigned NE = EndInstr->getNumOperands();
  unsigned NumUpper = 0;
  MachineOperand MoUpper = UpB, MoStep = Step;

  bool UpperIsImm = MoUpper.isImm();
  for (unsigned I = 0; I < NE; I++) {
    MachineOperand &MO = EndInstr->getOperand(I);
    if (MO.isReg() && MO.getReg() == TST->getHWLoopFinalReg()) {
      NumUpper = I;
      break;
    }
  }
  if (NumUpper > 0) { // will be deprecated in LLVM 12
    // NumUpper - must be register for upper
    if (NumUpper != NE - 1) { // extra args for upper and Step
      NumUpper++;
      (UpperIsImm ? MoStep : MoUpper) = EndInstr->getOperand(NumUpper);

      if (NumUpper != NE - 1) {
        NumUpper++;
        MoStep = EndInstr->getOperand(NumUpper);
        assert(MoStep.isReg());
      }
    }
  }
  MachineBasicBlock::iterator NMIa = BBLoop->instr_end();
  const MCInstrDesc &DeskAdd =
      TII->get(Step.isImm() ? TPC::ADDsip : TPC::ADDssp);
  MIB = BuildMI(*BBLoop, NMIa, StartInstr->getDebugLoc(), DeskAdd, LoopCount);
  MIB.addReg(LoopCount);
  if (MoStep.isReg()) {
    MIB.addReg(MoStep.getReg());
  } else {
    MIB.addImm(MoStep.getImm());
  }
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(LoopCount, RegState::Undef);
  MIB.addReg(TPC::SPRF_TRUE);
  MIB.addImm(0);

  HWLoops[L].LoopRegInstr.insert(MIB.getInstr());

  // CMP now

  CmpCode = getCMPCode(UpB, CmpK);
  MIB = BuildMI(*BBLoop, NMIa, StartInstr->getDebugLoc(), TII->get(CmpCode),
                RegCmp);
  MIB.addReg(LoopCount);
  if (!UpperIsImm) {
    MIB.addReg(MoUpper.getReg());
  } else {
    MIB.addImm(UpB.getImm());
  }
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(RegCmp, RegState::Undef);
  MIB.addReg(TPC::SPRF_TRUE);
  MIB.addImm(0);

  HWLoops[L].LoopRegInstr.insert(MIB.getInstr());
  // JMP now
  MIB = BuildMI(*BBLoop, NMIa, StartInstr->getDebugLoc(), TII->get(TPC::JMPR));
  MIB.addMBB(Loop1stBB);
  MIB.addReg(RegCmp, RegState::Kill);
  MIB.addImm(0);

  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  auto *const AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto *const MBPI = &getAnalysis<MachineBranchProbabilityInfo>();
  TPCPacketizerList Packetizer(*MF, MLI, AA, MBPI);
  Packetizer.PacketNum = 0;
  Packetizer.PacketizeMIs(BBPre, BBPre->begin(), BBPre->end());

  for (MachineInstr &MI : *BBPre) {
    if (MI.isBundle() && isWithLoopReg(MI, *TST))
      HWLoops[L].LoopRegInstr.insert(&MI);
  }

  Packetizer.PacketizeMIs(BBLoop, BBLoop->begin(), BBLoop->end());
  for (MachineInstr &MI : *BBLoop) {
    if (MI.isBundle() && isWithLoopReg(MI, *TST)) {
      unsigned LI = getLoopRefOrDef(HWLoops[L].LastClosedSon, L);
      HWLoops[LI].LoopRegInstr.insert(&MI);
    }
  }

  EndInstr->removeFromParent();
  StartInstr->removeFromParent();
}

void TPCUnHardwareLoops::checkDefUsedInLoopTunes(const MachineOperand &MO,
                                                 const unsigned L) {
  if (L == EMPTYREF)
    return;

#if 1 // need to hide issue G-1926
  const MachineOperand &Bound = HWLoops[L].StartInstr->getOperand(1);
  const MachineOperand &IncOp = HWLoops[L].StartInstr->getOperand(2);
  if ((Bound.isReg() && Bound.getReg() == MO.getReg()) ||
      (IncOp.isReg() && IncOp.getReg() == MO.getReg()))
    HWLoops[L].Spoiled = true;
#endif
  checkDefUsedInLoopTunes(MO, HWLoops[L].UpLoop);
}

void TPCUnHardwareLoops::markTreeAsSpoiled(const unsigned RootL) {
  if (RootL == EMPTYREF)
    return;
  HWLoops[RootL].Spoiled = true;
  markTreeAsSpoiled(HWLoops[RootL].DownLoop);  // The eldest son.
  markTreeAsSpoiled(HWLoops[RootL].RightLoop); // Brother.
}

void TPCUnHardwareLoops::escalateSpoiled() {
  assert(!HWLoops.empty());

  // no registers for counts comparing
  // will not be transformed until sp speel will be supported
  if (OccupiedSPRegNumbers.all())
    HWLoops[0].Spoiled = true;

  for (unsigned L = 0; L != EMPTYREF; L = HWLoops[L].RightLoop) {
    if (isLoopTreeSpoiled(L))
      markTreeAsSpoiled(L);
  }
}

void TPCUnHardwareLoops::processNotLoopInstr(MachineInstr &MI) {
  for (MachineOperand &MO : MI.operands()) {
    if (MO.isReg()) {
      int SPNum = getSPNumber(MO.getReg());
      if (SPNum >= 0) {
        if (MO.isKill())
          OccupiedSPRegNumbers.set(SPNum, false);
        if (MO.isDef())
          OccupiedSPRegNumbers.set(SPNum, true);
      }
      if (MO.isDef()) {
        // need to check if this def is in used in loop instruction
        // (init,upper,step) must not be so if HW looping was correct
        checkDefUsedInLoopTunes(MO, CurrentTopLoop);
      }
    }
  }
}

bool TPCUnHardwareLoops::isLoopTreeSpoiled(const unsigned RootL) {
  if (RootL == EMPTYREF)
    return false;
  return HWLoops[RootL].Spoiled ||                     // Self.
         isLoopTreeSpoiled(HWLoops[RootL].DownLoop) || // The eldest son.
         isLoopTreeSpoiled(HWLoops[RootL].RightLoop);  // Brother.
}

void TPCUnHardwareLoops::markTreeToTransformUp(unsigned L) {
  for (; L != EMPTYREF; L = HWLoops[L].UpLoop)
    HWLoops[L].Transform = true;
}

unsigned TPCUnHardwareLoops::reachRight(unsigned LeftMost) {
  unsigned L = LeftMost;
  for (; LeftMost != EMPTYREF; LeftMost = HWLoops[LeftMost].RightLoop)
    L = LeftMost;
  return L;
}

unsigned TPCUnHardwareLoops::findElderBrother(const unsigned EndLoop) {
  const unsigned Parent = HWLoops[EndLoop].UpLoop;
  const unsigned Left = Parent != EMPTYREF ? HWLoops[Parent].DownLoop : 0;
  return Left == EndLoop ? EMPTYREF : reachRight(Left);
}

void TPCUnHardwareLoops::setRight(const unsigned EndLoop) {
  unsigned Brother = findElderBrother(EndLoop);
  if (Brother != EMPTYREF)
    HWLoops[Brother].RightLoop = EndLoop;
}

void TPCUnHardwareLoops::processStartLoop(MachineInstr &MI,
                                          const unsigned MICount) {
  const unsigned LastIx = HWLoops.size();

  TLoopFrame LF;
  LF.StartInstr = &MI;
  LF.LoopCounter = MI.getOperand(MI.getNumExplicitOperands()).getReg();
  LF.InstrNumber = MICount;
  LF.Spoiled = false;
  LF.RightLoop = EMPTYREF;
  LF.UpLoop = CurrentTopLoop;
  LF.DownLoop = EMPTYREF;
  LF.LastClosedSon = EMPTYREF;
  if (CurrentTopLoop != EMPTYREF &&
      HWLoops[CurrentTopLoop].DownLoop == EMPTYREF) {
    HWLoops[CurrentTopLoop].DownLoop = LastIx;
  }
  if (isWithLoopReg(MI, *TST)) {
    LF.LoopRegInstr.insert(&MI);
  }
  HWLoops.push_back(LF);
  setRight(LastIx);

  CurrentTopLoop = LastIx;

  const unsigned Grand = HWLoops[CurrentTopLoop].UpLoop;
  if (Grand != EMPTYREF)
    HWLoops[Grand].LastClosedSon = EMPTYREF;
}

unsigned TPCUnHardwareLoops::processEndLoop(MachineInstr &MI,
                                            const unsigned MICount) {
  const unsigned LI = CurrentTopLoop;
  HWLoops[LI].EndInstr = &MI;
  HWLoops[LI].InstrNumber = MICount - HWLoops[LI].InstrNumber + 1;
  if (OccupiedSPRegNumbers.all()) {
    HWLoops[LI].Spoiled = true;
  } else {
    // Plan occupation of the first free SP register
    unsigned SPNumber = OccupiedSPRegNumbers.size() - 1;
    while (OccupiedSPRegNumbers.test(SPNumber))
      --SPNumber;
    HWLoops[LI].SPRegNum = SPNumber;
    OccupiedSPRegNumbers.set(SPNumber, true);
  }
  if (HWLoops[LI].Spoiled) {
    for (unsigned L = LI; L != EMPTYREF; L = HWLoops[L].UpLoop)
      HWLoops[L].Spoiled = true;
  }
  CurrentTopLoop = HWLoops[LI].UpLoop;
  for (unsigned P = CurrentTopLoop; P != EMPTYREF; P = HWLoops[P].UpLoop) {
    if (HWLoops[P].LastClosedSon == EMPTYREF)
      HWLoops[P].LastClosedSon = LI;
  }
  return LI;
}

bool TPCUnHardwareLoops::isTopMostHWLoop(const unsigned L) const {
  // Loop L should stay in hardware form.
  if (HWLoops[L].Transform)
    return false;

  // All parent loops should be transformed to software form.
  for (unsigned P = HWLoops[L].UpLoop; P != EMPTYREF; P = HWLoops[P].UpLoop) {
    if (!HWLoops[P].Transform)
      return false;
  }

  return true;
}

void TPCUnHardwareLoops::setHWCount(const unsigned Root,
                                    const Register Counter) {
  assert(!HWLoops[Root].Transform);
  assert(TST->isHWLoopReg(Counter));
  HWLoops[Root].NewLoopCounter = Counter;
  for (auto L = HWLoops[Root].DownLoop; L != EMPTYREF; L = HWLoops[L].RightLoop)
    setHWCount(L, Register(Counter.id() + 1));
}

void TPCUnHardwareLoops::setSoftCounts(const unsigned RootFrom,
                                       const Register Counter) {
  for (unsigned L = RootFrom; L != EMPTYREF; L = HWLoops[L].RightLoop) {
    if (HWLoops[L].Transform) {
      HWLoops[L].NewLoopCounter = Counter;
      setSoftCounts(HWLoops[L].DownLoop, Register(Counter.id() - 1));
    }
  }
}

void TPCUnHardwareLoops::setNewCounts() {
  assert(!HWLoops.empty());

  for (unsigned LI = 0; LI < HWLoops.size(); ++LI) {
    if (isTopMostHWLoop(LI))
      setHWCount(LI, TST->getHWLoopStartReg());
  }
  setSoftCounts(0, TST->getHWLoopFinalReg());
}

static void updateLoopRegisters(MachineInstr &MI, const TPCSubtarget &ST,
                                const SmallVector<Register, 4> &TransReg) {
  const Register StartHWLoopReg = ST.getHWLoopStartReg();
  const unsigned NumOps = isLoopInstr(MI) ? 3 : MI.getNumOperands();
  for (unsigned I = 0; I < NumOps; I++) {
    MachineOperand &MO = MI.getOperand(I);
    if (MO.isReg() && ST.isHWLoopReg(MO.getReg()))
      MO.setReg(TransReg[MO.getReg().id() - StartHWLoopReg.id()]);
  }
}

void TPCUnHardwareLoops::replaceCounts(CountersReplacementCtx &Ctx,
                                       unsigned RootL) {
  for (; RootL != EMPTYREF; RootL = HWLoops[RootL].RightLoop) {
    Ctx.TransReg.push_back(HWLoops[RootL].NewLoopCounter);
    for (MachineInstr *MI : HWLoops[RootL].LoopRegInstr) {
      if (!Ctx.UpdatedMIs.contains(MI)) {
        updateLoopRegisters(*MI, *TST, Ctx.TransReg);
        Ctx.UpdatedMIs.insert(MI);
      }
      if (MI->isBundle()) {
        const MachineBasicBlock *MBB = MI->getParent();
        for (auto MII = std::next(MI->getIterator());
             MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
          MachineInstr &BMI = *MII;
          if (!Ctx.UpdatedMIs.contains(&BMI)) {
            updateLoopRegisters(BMI, *TST, Ctx.TransReg);
            Ctx.UpdatedMIs.insert(&BMI);
          }
        }
      }
    }
    replaceCounts(Ctx, HWLoops[RootL].DownLoop);
    Ctx.TransReg.pop_back();
  }
}

void TPCUnHardwareLoops::constructHWLoopsTree() {
  HWLoops.clear();

  OccupiedSPRegNumbers.reset();
  // SP0 is reserved register before doron1+
  // SP0 is ordinary register since  doron1+
  if (!TST->getFeatureBits()[TPC::FeatureDoron1])
    OccupiedSPRegNumbers.set(0, true);

  unsigned MICount = 0;
  unsigned LastLoop = EMPTYREF;
  for (MachineBasicBlock &MBB : *MF) {
    for (MachineInstr &MI : MBB) {
      MICount++;

      if (isLoopInstr(MI)) {
        processStartLoop(MI, MICount);
        continue;
      }

      if (MI.getOpcode() == TPC::LOOPEND) {
        LastLoop = processEndLoop(MI, MICount);
        continue;
      }

      processNotLoopInstr(MI);
      if (isWithLoopReg(MI, *TST)) {
        unsigned Loop = CurrentTopLoop != EMPTYREF ? CurrentTopLoop : LastLoop;
        if (Loop != EMPTYREF) {
          unsigned LS = getLoopRefOrDef(HWLoops[Loop].LastClosedSon, Loop);
          HWLoops[LS].LoopRegInstr.insert(&MI);
        }
      }
    }
  }

  LLVM_DEBUG({
    dbgs() << "\nConstructed HW loops tree:\n";
    dumpHWLoops();
    dbgs() << "\n";
  });
}

void TPCUnHardwareLoops::markLoopsToUnHardware() {
  if (HWLoops.empty())
    return;

  escalateSpoiled();

  const unsigned MICountLimit =
      ForceUnHardwareLoops ? 0 : UnHardwareInstructionsMinCount.getValue();

  for (unsigned L = 0; L < HWLoops.size(); ++L) {
    if (HWLoops[L].InstrNumber >= MICountLimit && !isLoopTreeSpoiled(L))
      markTreeToTransformUp(L);
  }

  LLVM_DEBUG({
    dbgs() << "\nHW loops tree after plan markup:\n";
    dumpHWLoops();
    dbgs() << "\n";
  });
}

bool TPCUnHardwareLoops::hasLoopToUnHardware() {
  return any_of(HWLoops, [](const TLoopFrame &LF) { return LF.Transform; });
}

void TPCUnHardwareLoops::unHardwareLoops() {
  assert(hasLoopToUnHardware());

  setNewCounts();
  for (unsigned I = 0; I < size(HWLoops); ++I) {
    if (HWLoops[I].Transform)
      transformHWLoopBack(I);
  }

  CountersReplacementCtx Ctx;
  replaceCounts(Ctx);
}

bool TPCUnHardwareLoops::runOnMachineFunction(MachineFunction &Func) {

  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnableTPCUnHardwareLoops)
    return false;

  MF = &Func;
  TST = &MF->getSubtarget<TPCSubtarget>();
  TII = MF->getSubtarget().getInstrInfo();

  LLVM_DEBUG({
    dbgs() << "Input function:\n";
    Func.dump();
  });

  constructHWLoopsTree();

  markLoopsToUnHardware();

  if (hasLoopToUnHardware()) {
    unHardwareLoops();

    LLVM_DEBUG({
      dbgs() << "Transformed function:\n";
      Func.dump();
    });

    return true;
  }
  return false;
}

void TPCUnHardwareLoops::dumpHWLoops() {
  if (!HWLoops.empty())
    dumpHWLoop(0, 2);
}

void TPCUnHardwareLoops::dumpHWLoop(unsigned L, unsigned Indent) {
  const TLoopFrame &F = HWLoops[L];
  dbgs().indent(Indent) << *F.StartInstr;
  dbgs().indent(Indent) << "Old Cnt = " << printReg(F.LoopCounter) << "\n";
  dbgs().indent(Indent) << "New Cnt = " << printReg(F.NewLoopCounter) << "\n";
  dbgs().indent(Indent) << "MICount = " << F.InstrNumber << "\n";
  dbgs().indent(Indent) << "Spoiled = " << F.Spoiled << "\n";
  dbgs().indent(Indent) << "Planned = " << F.Transform << "\n";

  if (HWLoops[L].DownLoop != EMPTYREF)
    dumpHWLoop(HWLoops[L].DownLoop, Indent + 2);

  if (HWLoops[L].RightLoop != EMPTYREF)
    dumpHWLoop(HWLoops[L].RightLoop, Indent);
}

//===- TPCUnHardwareLoop.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-Transform HW Loop in usual loop------------------------------------===//
//
//===-if it exceeds limit ----------------------------------------------===//

#include "llvm/ADT/SmallSet.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "TPCVLIWPacketizer.h"
#include "llvm/InitializePasses.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "TPCVLIWPacketizer.h"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCUnHardwareLoops();
void initializeTPCUnHardwareLoopsPass(PassRegistry&);
}

static const char PassDescription[] = "TPC UnHardware Loops";
static const char PassName[] = "tpc-unhardware-loops";

// Flag to disable register balancing.
static cl::opt<bool>
EnableTPCUnHardwareLoops(PassName,
             cl::desc("transform hardware loops in usual loops"),
             cl::init(true), cl::Hidden);

static cl::opt<bool>  // debugging purposes
    ForceUnHardwareLoops("tpc-force-unhardware-loops",
                             cl::desc("transform hardware lops in usual loops of any size"),
                             cl::init(false), cl::Hidden);

#define EMPTYREF (unsigned)(~0)
#define BUNDLE_EXCEED_LIMIT 1950
#define PRECOUNT 4 // operand number before loop counts



namespace {
class TPCUnHardwareLoops : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  unsigned NumReplaced = 0;

public:
  static char ID;
  TPCUnHardwareLoops() : MachineFunctionPass(ID) {
    initializeTPCUnHardwareLoopsPass(*PassRegistry::getPassRegistry());
    HII = nullptr;
    HRI = nullptr;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<MachineBranchProbabilityInfo>();
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    AU.addPreserved<MachineDominatorTree>();
    AU.addPreserved<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  StringRef getPassName() const override { return PassDescription; }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:

  typedef struct {
    MachineInstr *StartInstr;
    MachineInstr *EndInstr;
    int LoopCount;
    int NewCount;
    unsigned nest;
    unsigned InstrNumber;
    unsigned sp;
    // Tree references
    unsigned UpLoop;
    unsigned RightLoop;
    unsigned DownLoop;
    unsigned LastClosedSon;
    SmallSet<MachineInstr*, 16> LoopRegInstr;
    bool spoiled;
    bool transform;
  } TLoopFrame;

  const TPCInstrInfo *HII;
  const TPCRegisterInfo *HRI;

  unsigned limitsize = BUNDLE_EXCEED_LIMIT; // may be corrected
  bool nomorelimitcorrection = false;
  unsigned CurrentTopLoop = EMPTYREF;
  unsigned SPScale;
  bool AtLeastOneLoopWasToTRansform = false;
  SmallVector<TLoopFrame, 64> hwloops;
  SmallSet<unsigned, 32> newroots;
  SmallSet<MachineInstr *, 16> AlreadyUpdated;

  void InitSPScale(void);
  bool SPScaleEmpty(void);
  void ExcludeSPReg(unsigned nreg);
  unsigned GiveSPScaleReg(void);
  void TakeSPScaleReg(unsigned nreg);
  bool TransformHWLoopBack(unsigned int, MachineFunction & MF);
  void CheckDefUsedInLoopTunes(MachineOperand &MO, unsigned curl);
  void ProcessNotLoopInstr(MachineInstr *MI);
  bool IsLoopTreeSpoiled(unsigned rooti);
  void MarkTreeAsSpoiled(unsigned rooti);
  void EscalateSpoiled(void);
  bool LoopToTransform(unsigned rooti);
  void MarkTreeToTransform(unsigned rooti);
  void MarkTreeToTransformUp(unsigned rooti);
  void EscalateTransform(void);
  void ProcessStartLoop(MachineInstr *MI, unsigned nest_count,
                        unsigned instr_count);
  unsigned ProcessEndLoop(MachineInstr *MI, unsigned instr_count);
  void SetRight(unsigned endloop);
  unsigned FindElderBrother(unsigned endloop);
  unsigned ReachRight(unsigned leftmost);
  void SetNewRoots(unsigned RootFrom);
  void SetNewCounts(void);
  void SetHWCount(unsigned root, int count);
  void SetSoftCounts(unsigned RootFrom,unsigned count);
  void ReplaceCounts(unsigned root);
};
}

char  TPCUnHardwareLoops::ID = 0;

INITIALIZE_PASS(TPCUnHardwareLoops, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCUnHardwareLoops() {
  return new TPCUnHardwareLoops();
}

static int getSPnumber(Register spr) { 
  int ret = -1;
  switch (spr) { 
  case TPC::SP0 : ret =  0; break;
  case TPC::SP1 : ret =  1; break;
  case TPC::SP2 : ret =  2; break;
  case TPC::SP3 : ret =  3; break;
  case TPC::SP4 : ret =  4; break;
  case TPC::SP5 : ret =  5; break;
  case TPC::SP6 : ret =  6; break;
  case TPC::SP7 : ret =  7; break;
  case TPC::SP8 : ret =  8; break;
  case TPC::SP9 : ret =  9; break;
  case TPC::SP10: ret = 10; break;
  case TPC::SP11: ret = 11; break;
  case TPC::SP12: ret = 12; break;
  case TPC::SP13: ret = 13; break;
  case TPC::SP14: ret = 14; break;
  case TPC::SP15: ret = 15; break;
  }
  return ret;
}

static Register getSPRegister(int nreg) {
  Register ret = 0;
  switch (nreg) { 
  case  0: ret = TPC::SP0  ; break;
  case  1: ret = TPC::SP1  ; break;
  case  2: ret = TPC::SP2  ; break;
  case  3: ret = TPC::SP3  ; break;
  case  4: ret = TPC::SP4  ; break;
  case  5: ret = TPC::SP5  ; break;
  case  6: ret = TPC::SP6  ; break;
  case  7: ret = TPC::SP7  ; break;
  case  8: ret = TPC::SP8  ; break;
  case  9: ret = TPC::SP9  ; break;
  case 10: ret = TPC::SP10 ; break;
  case 11: ret = TPC::SP11 ; break;
  case 12: ret = TPC::SP12 ; break;
  case 13: ret = TPC::SP13 ; break;
  case 14: ret = TPC::SP14 ; break;
  case 15: ret = TPC::SP15 ; break;
  }
  return ret;
}

static int SRegToNumber(Register sr) {
  int ret = -1;
  switch (sr) {
  case TPC::S32: ret = 32; break;
  case TPC::S33: ret = 33; break;
  case TPC::S34: ret = 34; break;
  case TPC::S35: ret = 35; break;
  }
  return ret;
}

static Register NumberToSReg(unsigned  sr) {
  int ret = -1;
  switch (sr) {
  case 32: ret = TPC::S32; break;
  case 33: ret = TPC::S33; break;
  case 34: ret = TPC::S34; break;
  case 35: ret = TPC::S35; break;
  }
  return ret;
}

static unsigned  GiveCMPCode(MachineOperand *loopupper, unsigned loopcmp) {
  bool isI = loopupper->isImm();
  switch (loopcmp) {
  case TPCII::LoopEQ:
      return (isI)?TPC::CMP_EQsip : TPC::CMP_EQssp;
    case TPCII::LoopNE:
      return (isI)?TPC::CMP_NEQsip : TPC::CMP_NEQssp;
    case TPCII::LoopLT:
      return (isI)?TPC::CMP_LESSsip : TPC::CMP_LESSssp;
    case TPCII::LoopLE:
      return (isI)?TPC::CMP_LEQsip : TPC::CMP_LEQssp;
    case TPCII::LoopGT:
      return (isI) ? TPC::CMP_GRTsip : TPC::CMP_GRTssp;
    case TPCII::LoopGE: {
      return (isI) ? TPC::CMP_GEQsip : TPC::CMP_GEQssp;
    }
    default :
      llvm_unreachable("Incorrect cmp mode");
    }
  return TPC::INSTRUCTION_LIST_END;
}

static bool isLoopInstr(MachineInstr *MI) {
  switch (MI->getOpcode()) {
  case TPC::LOOP1iiip :
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


static bool isPrLoopInstr(MachineInstr *MI) {
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

static bool MOPIsLoopRegUse(MachineOperand *operand) { 
  if (operand->isReg() && operand->isUse()) {
    Register lreg = operand->getReg();
    return SRegToNumber(lreg) > 0;
  }
  return false;
}


static bool InstrWithLoopReg(MachineInstr *instr) {
  unsigned nop = instr->getNumOperands();
  unsigned opc = instr->getOpcode();
  if (opc == TPC::LOOPEND) {
    return false;
  }
  if (isLoopInstr(instr)) {
    nop = 3;
  }
  for (unsigned int i = 0; i < nop; i++) {
    if (MOPIsLoopRegUse(&instr->getOperand(i))) {
      return true;
    }
  }
  return false;
}

void TPCUnHardwareLoops::InitSPScale(void) { 
  SPScale = 0xfffe; // from sp1 to sp15
}

bool TPCUnHardwareLoops::SPScaleEmpty(void) {
  return SPScale == 0; 
}


void TPCUnHardwareLoops::ExcludeSPReg(unsigned nreg) { 
  SPScale = ~(1 << nreg) & SPScale; 
}

unsigned TPCUnHardwareLoops::GiveSPScaleReg(void) { 
  int nreg = -1;
  unsigned ws = SPScale;
  while (ws) {
    ws = ws >> 1;
    nreg++;
  }
  assert(nreg >= 0 && "No more SP regs,Think about SP speel");
  ExcludeSPReg(nreg);
  return nreg;
}

void TPCUnHardwareLoops::TakeSPScaleReg(unsigned nreg) { 
  SPScale |= (1 << nreg); 
}


bool  TPCUnHardwareLoops::TransformHWLoopBack(unsigned int i_loop,
                                             MachineFunction &Func) {
  TLoopFrame *lf = &hwloops[i_loop];
  if (lf->spoiled || !lf->transform) {
    return false;
  }
  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  MachineInstr *StartInstr = lf->StartInstr;
  MachineInstr *EndInstr = lf->EndInstr;
  Register LoopCount = NumberToSReg(lf->LoopCount);
  auto lri = hwloops[i_loop].LoopRegInstr;
  if (lri.find(StartInstr) != lri.end()) {
    lri.erase(StartInstr);
  }

  Register regcmp;
  MachineBasicBlock *bbloop = EndInstr->getParent();
  MachineBasicBlock* AfterLoop  = bbloop->getNextNode();
  MachineBasicBlock *bbpre = StartInstr->getParent();
  MachineBasicBlock *Loop1stBB = bbpre->getNextNode();
  MachineOperand init = StartInstr->getOperand(0);
  MachineOperand upb = StartInstr->getOperand(1);
  MachineOperand step = StartInstr->getOperand(2);
  unsigned cmpk = StartInstr->getOperand(3).getImm();
  MachineInstrBuilder MIB;
  MachineBasicBlock::iterator nmis = bbpre->instr_end();

  AfterLoop->setLabelMustBeEmitted();
  bbloop->setLabelMustBeEmitted();
  if (isPrLoopInstr(StartInstr)) {
    MachineOperand LoopPred = StartInstr->getOperand(5);
    MachineOperand LoopPolar = StartInstr->getOperand(6);
    bool polar = LoopPolar.getImm();
    // Jump due to loop predicate
    MIB = BuildMI(*bbpre, nmis, StartInstr->getDebugLoc(), TII->get(TPC::JMPR));
    MIB.addMBB(AfterLoop);
    MIB.add(LoopPred);
    MIB.addImm(!polar);
  }

  const MCInstrDesc &DeskMOVinit =
      TII->get(init.isImm() ? TPC::MOVsip : TPC::MOVssp);
  MIB = BuildMI(*bbpre, nmis, StartInstr->getDebugLoc(), DeskMOVinit,
                LoopCount);
  MIB.add(init);
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(LoopCount, RegState::Undef);
  MIB.addReg(TPC::SP0);
  MIB.addImm(0);

  hwloops[i_loop].LoopRegInstr.insert(MIB.getInstr());

  // CMP now
  regcmp = getSPRegister(lf->sp);
  if (!regcmp)
  assert(regcmp);

  unsigned cmpcode = GiveCMPCode(&upb, cmpk);
  MIB = BuildMI(*bbpre, nmis, StartInstr->getDebugLoc(), TII->get(cmpcode),
                regcmp);
  MIB.addReg(LoopCount);
  MachineOperand hereUpb = upb;
  if (hereUpb.isReg()) {
    hereUpb.setIsKill(false);
  }
  MIB.add(hereUpb);
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(regcmp, RegState::Undef);
  MIB.addReg(TPC::SP0);
  MIB.addImm(0);

  hwloops[i_loop].LoopRegInstr.insert(MIB.getInstr());

  // JMP now
  unsigned jump_polarity = 1; 
  MIB = BuildMI(*bbpre, nmis, StartInstr->getDebugLoc(), TII->get(TPC::JMPR));
  MIB.addMBB(AfterLoop);
  MIB.addReg(regcmp, RegState::Kill);
  MIB.addImm(jump_polarity);

  regcmp = getSPRegister(lf->sp);
  assert(regcmp);

 // need to take right upper and step
  unsigned ne = EndInstr->getNumOperands();
  unsigned num_upper = 0;
  MachineOperand MoUpper=upb, MoStep=step;
 
  bool upper_is_imm = MoUpper.isImm();
  for (unsigned i = 0; i < ne; i++) {
    MachineOperand MO = EndInstr->getOperand(i);
    if (MO.isReg() && MO.getReg() == TPC::S35) {
      num_upper = i;
      break;
    }
  }
  // num_upper - must be register for upper

  if (num_upper != ne - 1) { // extra args for upper and step
    num_upper++;
    MachineOperand MOup = EndInstr->getOperand(num_upper);
    if (upper_is_imm) { // MOup is step
      MoStep = MOup;
    } 
    else {
      MoUpper = MOup;
    }

    if (num_upper == ne - 1) { // no step arg, nothin to do
    } else {
      num_upper++;
      MoStep = EndInstr->getOperand(num_upper);
      assert(MoStep.isReg());
    }
  }

  MachineBasicBlock::iterator nmia = bbloop->instr_end();
  const MCInstrDesc &DeskAdd =
      TII->get(step.isImm() ? TPC::ADDsip : TPC::ADDssp);
  MIB = BuildMI(*bbloop, nmia, StartInstr->getDebugLoc(), DeskAdd,
                LoopCount);
  MIB.addReg(LoopCount);
  if (MoStep.isReg()) {
    MIB.addReg(MoStep.getReg());
  } else {
    MIB.addImm(MoStep.getImm());
  }
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(LoopCount, RegState::Undef);
  MIB.addReg(TPC::SP0);
  MIB.addImm(0);

  hwloops[i_loop].LoopRegInstr.insert(MIB.getInstr());

  // CMP now
 
  cmpcode = GiveCMPCode(&upb, cmpk);
  MIB = BuildMI(*bbloop, nmia, StartInstr->getDebugLoc(), TII->get(cmpcode),
                regcmp);
  MIB.addReg(LoopCount);
  if (!upper_is_imm) {
    MIB.addReg(MoUpper.getReg());
  } else {
    MIB.addImm(upb.getImm());
  }
  MIB.addImm(TPCII::OpType::INT32);
  MIB.addImm(0);
  MIB.addReg(regcmp, RegState::Undef);
  MIB.addReg(TPC::SP0);
  MIB.addImm(0);

  auto cmpi = MIB.getInstr();
  hwloops[i_loop].LoopRegInstr.insert(cmpi);
  // JMP now
  MIB = BuildMI(*bbloop, nmia, StartInstr->getDebugLoc(),
                TII->get(TPC::JMPR));
  MIB.addMBB(Loop1stBB);
  MIB.addReg(regcmp, RegState::Kill);
  MIB.addImm(0);




  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  auto *AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto *MBPI = &getAnalysis<MachineBranchProbabilityInfo>();
  TPCPacketizerList Packetizer(Func, MLI, AA, MBPI);
  Packetizer.PacketNum = 0;
  MachineBasicBlock *lu = bbpre;
  Packetizer.PacketizeMIs(lu, lu->begin(), lu->end());

  for (MachineBasicBlock::iterator mi = lu->begin(), me = lu->end();
       mi != me;) {
    MachineBasicBlock::iterator nmi = std::next(mi);
    MachineInstr *MI = &*mi;
    if (MI->isBundle() && InstrWithLoopReg(MI)) {
      hwloops[i_loop].LoopRegInstr.insert(MI);
    }
    mi = nmi;
  }
 
  MachineBasicBlock *lb = bbloop;
  Packetizer.PacketizeMIs(lb, lb->begin(), lb->end());
  for (MachineBasicBlock::iterator mi = lb->begin(), me = lb->end();
       mi != me;) {
    MachineBasicBlock::iterator nmi = std::next(mi);
    MachineInstr *MI = &*mi;
    if (MI->isBundle() && InstrWithLoopReg(MI)) {
      unsigned last_innermost;
      last_innermost = hwloops[i_loop].LastClosedSon;
      if (last_innermost == EMPTYREF) {
        last_innermost = i_loop;
      }
      hwloops[last_innermost].LoopRegInstr.insert(MI);
    }
    mi = nmi;
  }
  EndInstr->removeFromParent();
  StartInstr->removeFromParent();

  return true;
}



void TPCUnHardwareLoops::CheckDefUsedInLoopTunes(MachineOperand &MO,
                                                 unsigned curl) {
  if (curl == EMPTYREF) {
    return;
  }
  TLoopFrame lf = hwloops[curl];
/*
*/
  // using is permissable if it is SAVE/RESTORE action

/* used register as count initializer can be used inside loop
  MachineOperand init = lf.StartInstr->getOperand(0);
  if (init.isReg() && !init.isKill() && init.getReg() == MO.getReg()) {
    hwloops[curl].spoiled = true;
    assert(0);
  }
*/
#if 1 // need to hide issue G-1926
  MachineOperand upb = lf.StartInstr->getOperand(1);
  MachineOperand step = lf.StartInstr->getOperand(2);
  if (upb.isReg() && upb.getReg() == MO.getReg()) {
    hwloops[curl].spoiled = true;
   // assert(0);
  }
  if (step.isReg() && step.getReg() == MO.getReg()) {
    hwloops[curl].spoiled = true;
   // assert(0);
  }
#endif
  unsigned upl = hwloops[curl].UpLoop;
  CheckDefUsedInLoopTunes(MO, upl);
}


void TPCUnHardwareLoops::ProcessNotLoopInstr(MachineInstr *MI) {
  for (MachineInstr::mop_iterator MOI = MI->operands_begin(),
                                  MOE = MI->operands_end();
       MOI != MOE; ++MOI) {
    MachineOperand &MO = *MOI;
    // skip loop instr for analysis
    if (MO.isReg()) {
      int spn = getSPnumber(MO.getReg());
      if (spn >= 0) {
        if (MO.isKill()) {
          TakeSPScaleReg(spn); // mark using SP register
        }
        if (MO.isDef()) {
          ExcludeSPReg(spn); // mark using SP register
        }
      }
      if (MO.isDef()) {
        // need to check if this def is in used in loop instruction
        // (init,upper,step) must not be so if HW looping was correct
        CheckDefUsedInLoopTunes(MO, CurrentTopLoop);
      }
    }
  }
}


bool TPCUnHardwareLoops::IsLoopTreeSpoiled(unsigned rooti) { 
  if (rooti == EMPTYREF) return false;
  bool spoiled = hwloops[rooti].spoiled;
  if (spoiled) return true;
  unsigned elderson = hwloops[rooti].DownLoop;
  unsigned brother = hwloops[rooti].RightLoop;
  spoiled |= IsLoopTreeSpoiled(elderson);
  spoiled |= IsLoopTreeSpoiled(brother);
  return spoiled;
}

void TPCUnHardwareLoops::MarkTreeAsSpoiled(unsigned rooti) {
  if (rooti == EMPTYREF) return;
  hwloops[rooti].spoiled = true;
  unsigned elderson = hwloops[rooti].DownLoop;
  unsigned brother = hwloops[rooti].RightLoop;
  MarkTreeAsSpoiled(elderson);
  MarkTreeAsSpoiled(brother);
}

void TPCUnHardwareLoops::EscalateSpoiled(void) {
  if (hwloops.empty())
      return;
  unsigned FirstLoop = 0;
  unsigned CurrTree = FirstLoop;
  if (SPScale == 0) {//no registers for counts comparing
    // will not be transforfed until sp speel will be supported
    hwloops[CurrTree].spoiled = true;
  }
  do {
    bool TreeSpoiled = IsLoopTreeSpoiled(CurrTree);
    if (TreeSpoiled) {
      MarkTreeAsSpoiled(CurrTree);
    }
    CurrTree = hwloops[CurrTree].RightLoop;
  } while (CurrTree != EMPTYREF);
}

bool TPCUnHardwareLoops::LoopToTransform(unsigned rooti) {
  if (rooti == EMPTYREF) return false;
  bool transform = hwloops[rooti].transform;
  if (transform)
    return true;
  unsigned elderson = hwloops[rooti].DownLoop;
  unsigned brother = hwloops[rooti].RightLoop;
  transform |= LoopToTransform(elderson);
  transform |= LoopToTransform(brother);
  return transform;
}

void TPCUnHardwareLoops::MarkTreeToTransform(unsigned rooti) {
    if (rooti == EMPTYREF) return;
  hwloops[rooti].transform = true;
  unsigned elderson = hwloops[rooti].DownLoop;
  unsigned brother = hwloops[rooti].RightLoop;
  MarkTreeToTransform(elderson);
  MarkTreeToTransform(brother);
}

void TPCUnHardwareLoops::MarkTreeToTransformUp(unsigned rooti) {
  if (rooti == EMPTYREF)
    return;
  hwloops[rooti].transform = true;
  unsigned father = hwloops[rooti].UpLoop;
  MarkTreeToTransformUp(father);
}

void TPCUnHardwareLoops::EscalateTransform(void) {
  if (hwloops.empty()) return;
  unsigned FirstLoop = 0;
  unsigned CurrTree = FirstLoop;
  do {
    if (LoopToTransform(CurrTree)) {
      MarkTreeToTransform(CurrTree);
    }
    CurrTree = hwloops[CurrTree].RightLoop;
  } while (CurrTree != EMPTYREF);
}

unsigned TPCUnHardwareLoops::ReachRight(unsigned leftmost) {
  unsigned lastnon = leftmost;
  while (leftmost != EMPTYREF) {
    lastnon = leftmost;
    leftmost = hwloops[leftmost].RightLoop;
  }
  return lastnon;
}

unsigned TPCUnHardwareLoops::FindElderBrother(unsigned endloop) {
  unsigned Parent = hwloops[endloop].UpLoop;
  unsigned left;
  if (Parent != EMPTYREF) {
    left = hwloops[Parent].DownLoop;
  } else { // top level
    left = 0;
  }
  if (left == endloop) {
    return EMPTYREF;
  }
  unsigned right = ReachRight(left);
  return right;
}

void TPCUnHardwareLoops::SetRight(unsigned endloop) {
  unsigned brother = FindElderBrother(endloop);
  if (brother != EMPTYREF) {
    hwloops[brother].RightLoop = endloop;
  }
}

void TPCUnHardwareLoops::ProcessStartLoop(MachineInstr *MI, unsigned nest_count,
                                          unsigned instr_count) {
  TLoopFrame lf;
  memset(&lf, 0, sizeof(lf));
  lf.StartInstr = MI;
  lf.nest = nest_count;
  if (isPrLoopInstr(MI))
    nest_count += 2;
  Register reg = MI->getOperand(nest_count).getReg();
  lf.LoopCount = SRegToNumber(reg);
 
  lf.InstrNumber = instr_count;
  lf.spoiled = false;
  lf.RightLoop = EMPTYREF;
  lf.UpLoop = EMPTYREF;
  lf.DownLoop = EMPTYREF;
  lf.LastClosedSon = EMPTYREF;
  unsigned lasti = hwloops.size();
  lf.UpLoop = CurrentTopLoop;
  if (CurrentTopLoop!=EMPTYREF &&
    hwloops[CurrentTopLoop].DownLoop == EMPTYREF) {
    hwloops[CurrentTopLoop].DownLoop = lasti;
  }
  if (InstrWithLoopReg(MI)) {
    lf.LoopRegInstr.insert(MI);
  }
  hwloops.push_back(lf);
  SetRight(lasti);

  CurrentTopLoop = lasti;
  unsigned int grand = hwloops[CurrentTopLoop].UpLoop;
  if (grand != EMPTYREF) {
    hwloops[grand].LastClosedSon = EMPTYREF;
  }
}

unsigned TPCUnHardwareLoops::ProcessEndLoop(MachineInstr *MI,
                                        unsigned instr_count) {
  unsigned inu;
  unsigned li = CurrentTopLoop; 
  hwloops[li].EndInstr = MI;
  hwloops[li].InstrNumber = inu = instr_count - hwloops[li].InstrNumber + 1;
  if (ForceUnHardwareLoops) {
    // correct limitsize for partial transformation
    if (limitsize == BUNDLE_EXCEED_LIMIT) {
      //not corrected yet
      if (inu < limitsize) {
        limitsize = inu;
      }
    }
  }
  if (inu >= limitsize) {
    MarkTreeToTransformUp(li); // now whole tree
    AtLeastOneLoopWasToTRansform = true;
  }
  if (SPScaleEmpty()) {
    hwloops[li].spoiled = true;
  } else {
    unsigned sp = GiveSPScaleReg();
    hwloops[li].sp = sp;
    TakeSPScaleReg(sp);
  }
  if (hwloops[li].spoiled) {
    inu = li;
    do{
      hwloops[inu].spoiled = true;
      inu = hwloops[inu].UpLoop;
    } while (inu != EMPTYREF);   
  }
  if (ForceUnHardwareLoops && !nomorelimitcorrection && hwloops[li].transform) {
    unsigned son = hwloops[li].DownLoop;
    if (son != EMPTYREF) {
      if (!hwloops[son].transform && hwloops[son].RightLoop==EMPTYREF) {
        if (hwloops[li].InstrNumber < 3*limitsize) {
          limitsize = hwloops[li].InstrNumber;
          hwloops[li].transform = false;
          nomorelimitcorrection = true;
        }
      }
    }
  }
  CurrentTopLoop = hwloops[li].UpLoop;
  unsigned int grand = CurrentTopLoop;
  while (grand != EMPTYREF) {
    if (hwloops[grand].LastClosedSon == EMPTYREF) {
      hwloops[grand].LastClosedSon = li;
    }
    grand = hwloops[grand].UpLoop;
  }
  return li;
}

void TPCUnHardwareLoops::SetNewRoots(unsigned RootFrom) {
  if (hwloops.empty())
    return;
  unsigned left = RootFrom;
  while (left != EMPTYREF) {
    if (hwloops[left].transform) {
      SetNewRoots(hwloops[left].DownLoop);
    } else {
      newroots.insert(left);
    }
    left = hwloops[left].RightLoop;
    SetNewRoots(left);
  }
}

void TPCUnHardwareLoops::SetHWCount(unsigned root, int count) { 
  if (root == EMPTYREF)  return;
  if (hwloops[root].transform)
  assert(!hwloops[root].transform);
  assert(count >= 32 && count <= 35);
  hwloops[root].NewCount = count;

  unsigned elderson = hwloops[root].DownLoop;
  if (elderson != EMPTYREF) {
    SetHWCount(elderson, count + 1);
    unsigned int brother = hwloops[elderson].RightLoop;
    while (brother != EMPTYREF) {
      if (hwloops[brother].transform) {
      } else {
        SetHWCount(brother, count + 1);
      }
      brother = hwloops[brother].RightLoop;
    }
  }
}

void TPCUnHardwareLoops::SetSoftCounts(unsigned RootFrom,unsigned count) {
  if (hwloops.empty())
    return;
  unsigned left = RootFrom;
  while (left != EMPTYREF) {
    if (hwloops[left].transform) {
      hwloops[left].NewCount = count;
      SetSoftCounts(hwloops[left].DownLoop,count-1);
    }
    left = hwloops[left].RightLoop;
    if (left != EMPTYREF && hwloops[left].transform) {
      SetSoftCounts(left, count);
    }
  }
}

void TPCUnHardwareLoops::SetNewCounts(void) {
  if (hwloops.empty())
    return;
  if (!newroots.empty()) {
    for (unsigned r : newroots) {
      SetHWCount(r, 32);
    }
  }
  SetSoftCounts(0,35);
}

static SmallVector<unsigned, 4> TransReg;

static void UpdateLoopRegisters(MachineInstr *MI) {
  unsigned nop = MI->getNumOperands();
  if (isLoopInstr(MI)) {
    nop = 3;
  }
  for (unsigned i = 0; i< nop; i++) {
    MachineOperand *MO = &MI->getOperand(i);
    if (MO->isReg()) {
      int lreg = SRegToNumber(MO->getReg());
      if (lreg >= 32 && lreg <= 35) {
        unsigned rel = lreg - 32;
        unsigned newcount = TransReg[rel];
        assert(newcount >= 32 && newcount <= 35);
        if ((unsigned)lreg != newcount) { // need to replace
          Register nreg = NumberToSReg(newcount);
          MO->setReg(nreg);
        }
      }
    }
  }
}


void TPCUnHardwareLoops::ReplaceCounts(unsigned root) {
  if (root == EMPTYREF)
    return;
  unsigned rl = root;
  if (hwloops[rl].spoiled)
    return;
  do {
    unsigned newcount = hwloops[rl].NewCount;
    if (!(newcount >= 32 && newcount <= 35))
    assert(newcount >= 32 && newcount <= 35);
    TransReg.push_back(newcount);
    for (auto MI : hwloops[rl].LoopRegInstr) {
      if (MI == nullptr)
        continue;
      if (AlreadyUpdated.find(MI) == AlreadyUpdated.end()) {
        UpdateLoopRegisters(MI);
        AlreadyUpdated.insert(MI);
      }
      if (MI->isBundle()) {
        const MachineBasicBlock *MBB = MI->getParent();
        MachineBasicBlock::instr_iterator MII = MI->getIterator();
        for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
          MachineInstr &BMI = *MII;
          MachineInstr *mi = &BMI;
          if (AlreadyUpdated.find(mi) == AlreadyUpdated.end()) {
            UpdateLoopRegisters(mi);
            AlreadyUpdated.insert(mi);
          }
        }
      }
    }
    ReplaceCounts(hwloops[rl].DownLoop);
    TransReg.pop_back();
    rl = hwloops[rl].RightLoop;
  } while (rl != EMPTYREF);

}

bool TPCUnHardwareLoops::runOnMachineFunction(MachineFunction &Func) {

  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnableTPCUnHardwareLoops)
    return false;

  hwloops.clear();
  newroots.clear();
  AlreadyUpdated.clear();
  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  HII = Func.getSubtarget<TPCSubtarget>().getInstrInfo();
  HRI = Func.getSubtarget<TPCSubtarget>().getRegisterInfo();
  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  auto *AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto *MBPI = &getAnalysis<MachineBranchProbabilityInfo>();
  TPCPacketizerList Packetizer(Func, MLI, AA, MBPI);
  NumReplaced = 0;
  MachineBasicBlock *MBB;
  unsigned instr_count = 0;
  unsigned nest_count = PRECOUNT;
  unsigned last_loop = EMPTYREF;
  
  InitSPScale(); 

  for (MachineFunction::iterator MBBI = MF->begin(), MBBE = MF->end();
       MBBI != MBBE; ++MBBI) {
    MBB = &*MBBI;
    for (MachineBasicBlock::iterator mi = MBB->begin(), me = MBB->end();
         mi != me;) {
      MachineBasicBlock::iterator nmi = std::next(mi);
      MachineInstr *MI = &*mi;
      instr_count++;
      auto opc = MI->getOpcode();
      if (isLoopInstr(MI)) {
        nest_count++;
        ProcessStartLoop(MI, nest_count, instr_count);
      }
      else if (opc == TPC::LOOPEND) {
        last_loop = ProcessEndLoop(MI, instr_count);
        nest_count--;
      } else {
        ProcessNotLoopInstr(MI);
        if (InstrWithLoopReg(MI)) {
          unsigned ls;
          if (CurrentTopLoop != EMPTYREF) {
            ls = hwloops[CurrentTopLoop].LastClosedSon;
            if (ls == EMPTYREF) {
              ls = CurrentTopLoop;
            }
            hwloops[ls].LoopRegInstr.insert(MI);
          } else if (last_loop != EMPTYREF) {
            ls = hwloops[last_loop].LastClosedSon;
            if (ls == EMPTYREF) {
              ls = last_loop;
            }
            hwloops[ls].LoopRegInstr.insert(MI);
          }              
        }
      }
      mi = nmi;
    }
  }
  assert(nest_count == PRECOUNT);
  if (AtLeastOneLoopWasToTRansform) {
    SetNewRoots(0);
    SetNewCounts();
    EscalateSpoiled();
    for (unsigned int i = 0; i < size(hwloops); i++) {
      if (TransformHWLoopBack(i, Func)) {
        NumReplaced++;
      }
    }
    if (NumReplaced > 0  && !hwloops.empty()) {
      ReplaceCounts(0);
    }
  }
  return NumReplaced > 0;
}

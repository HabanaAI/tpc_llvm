//===---- TPCImmToReg.cpp ------------===//
//===---------------------------------===//
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/Dominators.h"
#include <tuple>
#include <unordered_map>

#define DEBUG_TYPE "imm2reg"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCImmToReg();
void initializeTPCImmToRegPass(PassRegistry &);
} // namespace llvm

static const char PassDescription[] = "Use REG instead of IMM";
static const char PassName[] = "tpc-imm-to-reg";

static cl::opt<bool> EnableTPCImmToReg("tpc-imm-to-reg",
                                       cl::desc(PassDescription),
                                       cl::init(false), cl::Hidden);

namespace {
class TPCImmToReg : public MachineFunctionPass {
private:
  MachineFunction *MF = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  unsigned NumReplaced = 0;
  using InstrVec = std::vector<MachineInstr *>;
  using ImmToInstrIterMap = std::unordered_map<int64_t, InstrVec>;
  using OpDetail = std::tuple<unsigned, unsigned, unsigned>;
  ImmToInstrIterMap immInstructions;
  std::map<unsigned, OpDetail> OpDetailMap;

private:
  void registerInstructionsForXform();
  bool costLessThanThreshold(InstrVec &vec);
  MachineBasicBlock *closestDominator(MachineDominatorTree *DT,
                                      std::set<MachineBasicBlock *> &blocks);
  MachineBasicBlock *computeDominator(MachineDominatorTree *DT, InstrVec &vec);
  void replaceUses(MachineFunction &Func, MachineBasicBlock *MBB,
                   MachineInstr *MI, unsigned imm, unsigned vreg);
  unsigned createReg(MachineFunction &Func, int64_t imm,
                     MachineBasicBlock *dom);
  bool isPotentialLoopStep(MachineInstr *MI);
  void collectImmUse(MachineInstr *MI);
  void xformImmUse(MachineFunction &Func);
  bool processImmOperand(MachineFunction &Func);
  void printImmUse();

public:
  static char ID;
  StringRef getPassName() const override { return PassDescription; }
  TPCImmToReg() : MachineFunctionPass(ID) {
    initializeTPCImmToRegPass(*PassRegistry::getPassRegistry());
    registerInstructionsForXform();
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTree>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};
} // namespace

char TPCImmToReg::ID = 0;

INITIALIZE_PASS_BEGIN(TPCImmToReg, PassName, PassDescription, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_END(TPCImmToReg, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCImmToReg() { return new TPCImmToReg(); }

#ifndef NDEBUG
static std::string getSimpleNodeLabelDebug(const MachineBasicBlock *node) {
  if (!node->getName().empty())
    return node->getName().str();

  std::string str;
  raw_string_ostream os(str);

  node->printAsOperand(os, false);
  return os.str();
}
#endif

bool TPCImmToReg::costLessThanThreshold(InstrVec &vec) {
  // very naive cost-model, works for now.
  // TODO : should we factor in the impact on live-ranges, register pressure
  return (vec.size() >= 3);
}

void TPCImmToReg::registerInstructionsForXform() {
  // register ADDsip instruction for transform for now.
  // once the pass is more mature and can be enabled by default
  // register more instructions.
  OpDetailMap[TPC::ADDsip] = std::make_tuple(TPC::ADDsip, // from
                                             TPC::ADDssp, // to
                                             2            // imm pos
  );
}

MachineBasicBlock *
TPCImmToReg::closestDominator(MachineDominatorTree *DT,
                              std::set<MachineBasicBlock *> &blocks) {
  if (blocks.empty())
    return nullptr;
  std::set<MachineBasicBlock *>::iterator iter = blocks.begin();
  MachineBasicBlock *dom = *iter;
  while (++iter != blocks.end()) {
    MachineBasicBlock *mBB = *iter;
    dom = mBB ? DT->findNearestCommonDominator(dom, mBB) : nullptr;
    if (!dom) {
      return nullptr;
    }
  }
  return dom;
}

MachineBasicBlock *TPCImmToReg::computeDominator(MachineDominatorTree *DT,
                                                 InstrVec &vec) {
  std::set<MachineBasicBlock *> ublocks;
  for (auto &instr : vec) {
    ublocks.insert(instr->getParent());
    LLVM_DEBUG(dbgs() << "block: "
                      << getSimpleNodeLabelDebug(instr->getParent()) << "\n");
  }
  MachineBasicBlock *dom = closestDominator(DT, ublocks);
  LLVM_DEBUG(dbgs() << "common dom: " << getSimpleNodeLabelDebug(dom) << "\n");
  if (ublocks.find(dom) != ublocks.end()) {
    LLVM_DEBUG(dbgs() << "dom is one of the blocks: "
                      << getSimpleNodeLabelDebug(dom) << "\n");
    auto domBB = DT->getNode(dom);
    if (domBB == nullptr) {
      dom = nullptr;
    } else {
      auto idom = domBB->getIDom();
      if (idom) {
        dom = idom->getBlock();
      } else {
        dom = nullptr;
      }
    }
  }
  return dom;
}

static bool getCmpMode(unsigned Opcode) {
  switch (Opcode) {
  case TPC::CMP_EQssp:
  case TPC::CMP_EQsip:
  case TPC::CMP_NEQssp:
  case TPC::CMP_NEQsip:
  case TPC::CMP_LESSssp:
  case TPC::CMP_LESSsip:
  case TPC::CMP_LEQssp:
  case TPC::CMP_LEQsip:
  case TPC::CMP_GRTssp:
  case TPC::CMP_GRTsip:
  case TPC::CMP_GEQssp:
  case TPC::CMP_GEQsip:
    return true;
  default:
    return false;
  }
}

bool TPCImmToReg::isPotentialLoopStep(MachineInstr *MI) {
  // check if the o/p of instruction is used in a cmp instruction.
  // if so, it is a potential incr.
  MachineOperand Out = MI->getOperand(0);
  if (Out.isReg()) {
    for (MachineRegisterInfo::use_iterator RSUse = MRI->use_begin(Out.getReg()),
                                           RSE = MRI->use_end();
         RSUse != RSE; ++RSUse) {
      MachineInstr *RSUseMI = RSUse->getParent();
      auto *I = dyn_cast<MachineInstr>(&*RSUseMI);
      if (I) {
        if (getCmpMode(I->getOpcode())) {
          return true;
        }
      }
    }
  }
  return false;
}

void TPCImmToReg::collectImmUse(MachineInstr *MI) {
  auto opc = MI->getOpcode();
  auto opd_it = OpDetailMap.find(opc);
  if ((opd_it == OpDetailMap.end()) || isPotentialLoopStep(MI)) {
    return;
  }
  MachineOperand opnd = MI->getOperand(std::get<2>(opd_it->second));
  if (opnd.isImm()) {
    int64_t imm = opnd.getImm();
    immInstructions[imm].push_back(MI);

    LLVM_DEBUG(dbgs() << "collected imm : " << imm << " from : ");
    LLVM_DEBUG(MI->dump());
  }
}

unsigned TPCImmToReg::createReg(MachineFunction &Func, int64_t imm,
                                MachineBasicBlock *dom) {
  MF = &Func;
  auto ST = &MF->getSubtarget();
  auto TII = ST->getInstrInfo();

  unsigned v_reg = MRI->createVirtualRegister(
      ST->getTargetLowering()->getRegClassFor(MVT::i32));
  MachineBasicBlock::iterator InsertPos = --(dom->end());
  BuildMI(*dom, InsertPos, DebugLoc(), TII->get(TPC::MOVsip), v_reg)
      .addImm(imm)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(v_reg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  return v_reg;
}

void TPCImmToReg::replaceUses(MachineFunction &Func, MachineBasicBlock *MBB,
                              MachineInstr *MI, unsigned imm, unsigned vreg) {
  MF = &Func;
  MRI = &MF->getRegInfo();
  auto ST = &MF->getSubtarget();
  auto TII = ST->getInstrInfo();
  bool bfound = false;
  unsigned immOpc = 0;
  auto opc = MI->getOpcode();
  auto opd_it = OpDetailMap.find(opc);
  if (opd_it != OpDetailMap.end()) {
    unsigned idx = std::get<2>(opd_it->second);
    auto &op = MI->getOperand(std::get<2>(opd_it->second));
    if (op.isImm() && op.getImm() == imm) {
      immOpc = std::get<1>(opd_it->second);
      bfound = true;
    }
    if (bfound) {
      MachineInstrBuilder MIB;
      MIB = BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(immOpc),
                    MI->getOperand(0).getReg());
      for (unsigned int i = 1; i < idx; i++) {
        MIB.addReg(MI->getOperand(i).getReg());
      }
      MIB.addReg(vreg);
      for (unsigned int i = idx + 1; i < MI->getNumOperands(); i++) {
        MIB.add(MI->getOperand(i));
      }
      LLVM_DEBUG(dbgs() << "replacing instruction using imm : " << imm
                        << " in : ");
      LLVM_DEBUG(MI->dump());
      MI->removeFromParent();
      ++NumReplaced;
    }
  }
}

void TPCImmToReg::xformImmUse(MachineFunction &Func) {
  MachineDominatorTree *DT = &getAnalysis<MachineDominatorTree>();
  for (auto &immInstr : immInstructions) {
    LLVM_DEBUG(dbgs() << "Processing instructions using imm : "
                      << immInstr.first << "\n");
    auto &instrVec = immInstr.second;
    if (costLessThanThreshold(instrVec)) {
      MachineBasicBlock *dom = computeDominator(DT, instrVec);
      if (dom) {
        LLVM_DEBUG(dbgs() << "dom: " << getSimpleNodeLabelDebug(dom) << "\n");
        unsigned vreg = createReg(Func, immInstr.first, dom);
        for (auto &instr : instrVec) {
          MachineInstr *MI = instr;
          replaceUses(Func, MI->getParent(), MI, immInstr.first, vreg);
        }
      } else {
        LLVM_DEBUG(
            dbgs() << "Common dominator not found for instructions using imm : "
                   << immInstr.first << "\n");
        continue;
      }
    }
  }
}

bool TPCImmToReg::processImmOperand(MachineFunction &Func) {
  for (auto &MBB : Func) {
    for (auto &MI : MBB) {
      collectImmUse(&MI);
    }
  }
  xformImmUse(Func);
  immInstructions.clear();
  return false;
}

bool TPCImmToReg::runOnMachineFunction(MachineFunction &Func) {
  if (!EnableTPCImmToReg)
    return false;
  MF = &Func;
  MRI = &MF->getRegInfo();
  if (skipFunction(Func.getFunction()))
    return false;
  processImmOperand(Func);

  return NumReplaced > 0;
}

#if 0
void  TPCImmToReg::printImmUse() {
  dbgs() << "printImmUse .." << "\n";
  //for(auto& x : immInstructions) {
  //  dbgs() << "Imm: " << x.first << "\n";
  //  auto& vec = x.second;
  //  for(auto& instr : vec) {
  //    MachineInstr* MI = instr.first;
  //    MI->dump(); 
  //  }	    
  //}
}
#endif

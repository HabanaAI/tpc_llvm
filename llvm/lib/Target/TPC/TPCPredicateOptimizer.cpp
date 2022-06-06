//===---- TPCPredicateOptimizer.cpp --- Optimizes predicates --------------===//
//
//===----------------------------------------------------------------------===//
//
// This pass:
// - replaces predicates with known value with UNPR.
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/Debug.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"

#define DEBUG_TYPE "tpc-pred"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCPredicateOptimizer();
void initializeTPCPredicateOptimizerPass(PassRegistry&);
}

static const char PassDescription[] = "TPC predicate optimizer";
static const char PassName[] = "tpc-pred";

// Flag to disable predicate optimization.
static cl::opt<bool>
EnablePredicateOptimizer("optimize-predicates",
                       cl::desc("Optimize use of TPC predicates (default=true)"),
                       cl::init(true), cl::Hidden);



namespace {
class TPCPredicateOptimizer : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  unsigned NumReplaced = 0;
  unsigned NumRemoved = 0;
  const TPCInstrInfo *TII = nullptr;
  const TargetRegisterInfo *TRI = nullptr;
  const TPCSubtarget *TST = nullptr;
  MachineRegisterInfo *MRI = nullptr;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCPredicateOptimizer() : MachineFunctionPass(ID) {
    initializeTPCPredicateOptimizerPass(*PassRegistry::getPassRegistry());
  }

  /// \brief Loop over all of the basic blocks, replacing predicated instructions
  /// by equivalent non-predicated instructions if needed and when possible.
  ///
  bool runOnMachineFunction(MachineFunction &MF) override;

  bool isSPRFPredicated(const MachineInstr &I, unsigned &PredRegLoc,
                        unsigned &IncomeValue);
  bool isDstFullyWritten(const MachineInstr &I);
};
}

char TPCPredicateOptimizer::ID = 0;

INITIALIZE_PASS(TPCPredicateOptimizer, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCPredicateOptimizer() {
  return new TPCPredicateOptimizer();
}


bool TPCPredicateOptimizer::isSPRFPredicated(const MachineInstr &I,
                                             unsigned &PredRegLoc,
                                             unsigned &IncomeValue) {
  bool Polarity;
  int Predicate;

  if (!looksLikeHavingAPredicate(I, Predicate, Polarity))
    return false;

  Register Reg = I.getOperand(Predicate).getReg();
  const TargetRegisterClass *RC = getRegisterClass(Reg, *MRI);
  if (RC != &TPC::SPRFRegClass)
    return false; //TODO VP0 ??

  PredRegLoc = Predicate;
  IncomeValue = ~0U;

  if (I.getDesc().getNumDefs() == 0)
    return true;
  
  int NIncome = getIncome(I);
  if (NIncome > 0) {
    IncomeValue = NIncome;
  } else {
    assert(TPCII::isStoreInst(I.getDesc()) && "Def under predicate must have income");
  }

  return true;
}

bool TPCPredicateOptimizer::isDstFullyWritten(const MachineInstr &I) {
  Register ireg = I.getOperand(0).getReg();
  const TargetRegisterClass *RC = getRegisterClass(ireg, *MRI);
  // Currently, we optimize only for SRF and VRF dest
  if (RC != &TPC::SRFRegClass && RC != &TPC::VRFRegClass) {
    return false;
  }
  if (TPCII::isVPUInst(I.getDesc())) {
    switch (TPCII::getSlotOpCode(I.getDesc())) {

    // Accumulators
    case TPCII::vpuMAC:
    case TPCII::vpuMSAC:
      return false;

    case TPCII::vpuCONVERT: {
      if (isAllLanesVpuConvert(I, *TST))
        return true;

      const unsigned SrcTy = I.getOperand(2).getImm();
      const unsigned DstTy = (getSwitches(I) >> 8) & 0xF;
      return TPCII::getOpTypeMinSizeBits(static_cast<TPCII::OpType>(DstTy)) >
             TPCII::getOpTypeMinSizeBits(static_cast<TPCII::OpType>(SrcTy));
    }
    case TPCII::vpuCONVERT_INT8:
    case TPCII::vpuCONVERT_UINT8:
      return false;
    case TPCII::vpuCONVERT_INT16:
    case TPCII::vpuCONVERT_UINT16:
      return isAllLanesVpuConvertI16(I, *TST);
    case TPCII::vpuCONVERT_INT32:
    case TPCII::vpuCONVERT_UINT32:
      return isAllLanesVpuConvertI32(I, *TST);
    case TPCII::vpuCALC_FP_SPECIAL:
      // CALC_FP_SPECIAL leaves DST unchanged for some input values
    case TPCII::vpuPACK:
    case TPCII::vpuUNPACK:
      // UNPACK requires zero initialization of the dst in TPC kernels
      return false;

    case TPCII::vpuMOV_DUAL_GROUP:
    {
      unsigned Sw = llvm::getSwitches(I);
      bool hasAllSw = (Sw & TPCII::SW_MDG_TYPE_MASK) == TPCII::SW_MDG_TYPE_ALL;

      if (!hasAllSw || (Sw & TPCII::SW_MDG_SWAP_TYPE)) {
        return false;
      }
      // Extract SrcC from Sw operand (sw{23-16} according to InstrFormat)
      unsigned SrcC = (Sw >> 16) & 0xFF;

      if (SrcC != 0xFF)
        return false;

      const MachineOperand &ImmOp = I.getOperand(2);
      unsigned Imm = ImmOp.getImm();
      if (!ImmOp.isImm()) {
        return false;
      }
      if (Imm != 0xFFFFFFFF) {
        return false;
      }
      return true;
    }
    case TPCII::vpuMOV_GROUP:
    {
      const MachineOperand &ImmOp = I.getOperand(2);
      if (!ImmOp.isImm()) {
        return false;
      }
      unsigned Imm = ImmOp.getImm();
      if (Imm != 0xFFFFFFFF) {
        return false;
      }
      const MachineOperand &SwOp = I.getOperand(3);
      if (!SwOp.isImm()) {
        return false;
      }
      unsigned Sw = SwOp.getImm();
      if ((Sw & 0x3F) != 0x3F) {
        return false;
      }
      return true;
    }
    case TPCII::vpuSHUFFLE:
      // TODO: it is required to recursively check operands for
      //       known values - it is probably better to do this
      //       at intrinsics level, using ValueTracking feature.
      return false;
    }
  }
  else if (TPCII::isSPUInst(I.getDesc())) {
    switch (TPCII::getSlotOpCode(I.getDesc())) {
    case TPCII::spuMAC:
    case TPCII::spuCONVERT:
    case TPCII::spuCALC_FP_SPECIAL:
      // CALC_FP_SPECIAL leaves DST unchanged for some input values
      return false;
    default:;
      return true;
    }
  }
  else  if (TPCII::isLoadInst(I.getDesc())) {
    // TODO
    return false;
  }
  else  if (TPCII::isStoreInst(I.getDesc())) {
    // TODO
    return false;
  }
  return true;
}

bool TPCPredicateOptimizer::runOnMachineFunction(MachineFunction &Func) {
  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnablePredicateOptimizer)
    return false;

  LLVM_DEBUG({
    dbgs() << "Input function:\n";
    Func.dump();
    dbgs() << "\n";
  });

  MF = &Func;
  TRI = MF->getSubtarget().getRegisterInfo();
  TII = MF->getSubtarget<TPCSubtarget>().getInstrInfo();
  TST = &MF->getSubtarget<TPCSubtarget>();
  MRI = &MF->getRegInfo();
  NumReplaced = NumRemoved = 0;

  for (auto &BB : Func) {
    for (auto IPtr = BB.begin(), E = BB.end(); IPtr != E;) {
      MachineInstr &I = *IPtr;
      ++IPtr;
      unsigned PredRegLoc;
      unsigned IncomeArg;
      if (isSPRFPredicated(I, PredRegLoc, IncomeArg)) {
        // Get polarity.
        bool InvertedPolarity = I.getOperand(PredRegLoc + 1).getImm() != 0;

        // Get predicate.
        const MachineOperand &PredOp = I.getOperand(PredRegLoc);
        assert(PredOp.isReg());
        unsigned PredReg = PredOp.getReg();

        // Try to evaluate predicate value.
        bool PredicateValue;
        if (PredReg == TPC::SPRF_TRUE) {
          PredicateValue = true;
        } else {
          // Get instruction that defines the predicate.
          MachineInstr* PredDef = MRI->getVRegDef(PredReg);
          unsigned DefiningValueNo; // N of immediate operand that defines the pred
          switch (PredDef->getOpcode()) {
          case TPC::COPY:
            DefiningValueNo = 1;
            break;
          case TPC::MOVpip: {
            bool Polarity;
            int Predicate;
            if (looksLikeHavingAPredicate(*PredDef, Predicate, Polarity)) {
              if (PredDef->getOperand(Predicate).getReg() == TPC::SPRF_TRUE) {
                DefiningValueNo = 1;
                break;
              }
            }
            LLVM_FALLTHROUGH;
          }
          default:
            continue;
          }
          if (PredDef->getOperand(DefiningValueNo).isReg()) {
            unsigned DefReg = PredDef->getOperand(DefiningValueNo).getReg();
            if (DefReg == TPC::SPRF_TRUE) {
              PredicateValue = true;
            } else {
              continue;
            }
          }
          else if (!PredDef->getOperand(DefiningValueNo).isImm())
            continue;
          else
            PredicateValue = PredDef->getOperand(DefiningValueNo).getImm() != 0;
        }

        // Predicate value evaluated. Transform the instruction.
        if (PredicateValue != InvertedPolarity) {
          // Replace register with UNPR
          LLVM_DEBUG(dbgs() << "del pred: " << I);
          I.getOperand(PredRegLoc).setReg(TPC::SPRF_TRUE);
          I.getOperand(PredRegLoc).setIsKill(false);
          I.getOperand(PredRegLoc + 1).setImm(0);
          LLVM_DEBUG(dbgs() << "      to: " << I);

          // TODO: Replace income operand with undef for scalar data but only
          // if it is not used as accumulator.
          if (I.getOperand(0).isReg() && I.getOperand(0).isDef() &&
              isDstFullyWritten(I)) {
            unsigned IncomeReg = I.getOperand(IncomeArg).getReg();
            MachineInstr* IncomeDef = MRI->getVRegDef(IncomeReg);
            if (IncomeDef->getOpcode() != TPC::IMPLICIT_DEF) {
              LLVM_DEBUG(dbgs() << "replace: " << I);
              Register ireg = I.getOperand(0).getReg();
              const TargetRegisterClass *RC = getRegisterClass(ireg, *MRI);
              unsigned Undef = MRI->createVirtualRegister(RC);
              const DebugLoc &DL = I.getDebugLoc();
              BuildMI(*(I.getParent()), &I, DL, TII->get(TPC::IMPLICIT_DEF), Undef);
              I.getOperand(IncomeArg).setReg(Undef);
              //I.getOperand(IncomeArg).setIsUndef();
              LLVM_DEBUG(dbgs() << "   with: " << I);
            }
          }
          ++NumReplaced;
        } else {
          // Remove instruction.
          LLVM_DEBUG(dbgs() << "remove: " << I);
          unsigned NIn = IncomeArg;
          for (MachineOperand &D : I.defs()) {
            assert(D.isReg());
            unsigned OldReg = D.getReg();
            unsigned NewReg = I.getOperand(NIn).getReg();
            MRI->replaceRegWith(OldReg, NewReg);
            ++NIn;
          }
          I.eraseFromParent();
          ++NumRemoved;
        }
      }
    }
  }

  const bool Changed = NumReplaced > 0 || NumRemoved > 0;

  LLVM_DEBUG(if (Changed) {
    dbgs() << "\nOutput function:\n";
    Func.dump();
  });

  return Changed;
}

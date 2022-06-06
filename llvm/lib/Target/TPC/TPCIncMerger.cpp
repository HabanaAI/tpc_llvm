//===---- TPCIncMerger.cpp --- Merge increments to other instructions -----===//
//
//===----------------------------------------------------------------------===//
//
// This pass:
// - merge combination of some instructions and subsequent increment into
//   combined instruction.
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/InitializePasses.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include <iostream>
#include <set>
using namespace llvm;

#define DEBUG_TYPE "tpc-merge"

namespace llvm {
FunctionPass *createTPCIncMerger();
void initializeTPCIncMergerPass(PassRegistry &);
}

static const char PassDescription[] = "TPC increment merger";
static const char PassName[] = "tpc-inc";

// Flag to disable predicate optimization.
static cl::opt<bool>
EnableIncMerger("tpc-inc-merger",
                cl::desc("Merge instructions with increments (default=true)"),
                cl::init(false), cl::Hidden);

static cl::opt<bool>
DeduceIncrement("tpc-deduce-inc",
                cl::desc("Deduce tensor type from read/write operation"),
                cl::init(true), cl::Hidden);


// Value = LD_G Address, Switches, Income, Predicate, Polarity
const unsigned N_Addr_in_LD_G = 1;
const unsigned N_Switches_in_LD_G = 2;

// Value = LD_GI Address, DimMask, Switches, Income, Predicate, Polarity)
const unsigned N_Addr_in_LD_GI = 1;
const unsigned N_Mask_in_LD_GI = 2;
const unsigned N_Switches_in_LD_GI = 3;

// Index2 = ADD Imm, Index1, Mask, DataType, Switches, Income, Predicate, Polarity
const unsigned N_Imm_in_ADD = 1;
const unsigned N_Mask_in_ADD = 3;

// Address = GEN_ADDR Tensor, Index, Switches, Income, Predicate, Polarity
const unsigned N_Tensor_in_GEN_ADDR = 1;
const unsigned N_Index_in_GEN_ADDR = 2;
const unsigned N_Switches_in_GEN_ADDR = 3;

// Value = LD_TNSR Index, Tensor, Switches, Income, Predicate, Polarity
const unsigned N_Index_in_LD_TNSR = 1;
const unsigned N_Switches_in_LD_TNSR = 3;

// ST_TNSR Index, Tensor, Value, Switches, Predicate, Polarity
const unsigned N_Index_in_ST_TNSR = 0;
const unsigned N_Switches_in_ST_TNSR = 3;

// ST_G Addr, Value, Switches:$sw, Predicate, Polarity
const unsigned N_Addr_in_ST_G = 0;
const unsigned N_Switches_in_ST_G = 2;

// PREFETCH Address, Switches, Predicate, Polarity
const unsigned N_Addr_in_PREFETCH = 0;
const unsigned N_Switches_in_PREFETCH = 1;


namespace {
enum class MergeKind {
  Nothing,
  Prefetch,
  LoadTensor,
  StoreTensor,
  LoadGlobal,
  LoadGlobalIRF,
  StoreGlobal
};

struct FoldableOperation {
  FoldableOperation() = default;
  FoldableOperation(MergeKind K) : Kind(K) {}

  MergeKind Kind = MergeKind::Nothing;
  SmallVector<MachineInstr *, 4> Source;
  MachineBasicBlock *BB = nullptr;
  int LoopDepth = -1;
  int64_t IncVal = 0;
  int64_t DimMask = 0;
};

class TPCIncMerger : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  MachineLoopInfo *MLI = nullptr;
  MachineDominatorTree *MDT = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TPCInstrInfo *TII = nullptr;
  const TPCSubtarget *ST = nullptr;
  std::set<MachineInstr *> InstrToRemove;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCIncMerger() : MachineFunctionPass(ID) {
    initializeTPCIncMergerPass(*PassRegistry::getPassRegistry());
  }

  /// \brief Loop over all of the basic blocks, replacing predicated instructions
  /// by equivalent non-predicated instructions if needed and when possible.
  ///
  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  bool isFoldableLoadGlobal(MachineInstr *MI, FoldableOperation &Operation);
  bool isFoldableStoreGlobal(MachineInstr *MI, FoldableOperation &Operation);
  bool isFoldablePrefetch(MachineInstr *MI, FoldableOperation &Operation);
  bool isFoldableLoadTensor(MachineInstr *MI, FoldableOperation &Operations);
  bool isFoldableSaveTensor(MachineInstr *MI, FoldableOperation &Operations);

  bool matchAutoIncPattern(MachineInstr *AccessMI, FoldableOperation &Operation);
  bool foldIndexIncrementInScalarOp(const FoldableOperation &Op);
};
}

char TPCIncMerger::ID = 0;

INITIALIZE_PASS_BEGIN(TPCIncMerger, PassName, PassDescription, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(TPCIncMerger, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCIncMerger() {
  return new TPCIncMerger();
}

raw_ostream &operator << (raw_ostream &out, MergeKind MK) {
  switch (MK) {
  case MergeKind::Nothing:
    out << "Nothing";
    break;
  case MergeKind::LoadTensor:
    out << "LoadTensor";
    break;
  case MergeKind::StoreTensor:
    out << "StoreTensor";
    break;
  case MergeKind::Prefetch:
    out << "Prefetch";
    break;
  case MergeKind::LoadGlobal:
    out << "LoadGlobal";
    break;
  case MergeKind::LoadGlobalIRF:
    out << "LoadGlobalIRF";
    break;
  case MergeKind::StoreGlobal:
    out << "StoreGlobal";
    break;
  }
  return out;
}

static bool is_ST_TNSR_with_auto_inc_form(MachineInstr *MI) {
  const MCInstrDesc &D = MI->getDesc();
  if (!TPCII::isStTnsr(D))
    return false;

  switch (MI->getOpcode()) {
  case TPC::ST_TNSR_PDvp:
  case TPC::ST_TNSR_PDmp:
  case TPC::ST_TNSR_PDRvp:

  case TPC::ST_TNSR_Dvp:
  case TPC::ST_TNSR_Dmp:
  case TPC::ST_TNSR_RDvp:

  case TPC::ST_TNSR_LOW_Dvp:
  case TPC::ST_TNSR_LOW_Dmp:
  case TPC::ST_TNSR_LOW_RDvp:

  case TPC::ST_TNSR_HIGH_Dvp:
  case TPC::ST_TNSR_HIGH_Dmp:
  case TPC::ST_TNSR_HIGH_RDvp:
    return false;
  }
  return true;
}

static bool is_LD_TNSR_with_auto_inc_form(MachineInstr *MI) {
  const MCInstrDesc &D = MI->getDesc();
  if (!TPCII::isLdTnsr(D, true))
    return false;

  switch (MI->getOpcode()) {
  case TPC::LD_TNSR_Dvp:
  case TPC::LD_TNSR_Dvm:
  case TPC::LD_TNSR_Dmp:
  case TPC::LD_TNSR_Dmm:

  case TPC::LD_TNSR_PDvp:
  case TPC::LD_TNSR_PDvm:
  case TPC::LD_TNSR_PDmp:
  case TPC::LD_TNSR_PDmm:

  case TPC::LD_TNSR_LOW_Dvp:
  case TPC::LD_TNSR_LOW_Dvm:
  case TPC::LD_TNSR_LOW_Dmp:
  case TPC::LD_TNSR_LOW_Dmm:

  case TPC::LD_TNSR_LOW_PDvp:
  case TPC::LD_TNSR_LOW_PDvm:
  case TPC::LD_TNSR_LOW_PDmp:
  case TPC::LD_TNSR_LOW_PDmm:

  case TPC::LD_TNSR_HIGH_Dvp:
  case TPC::LD_TNSR_HIGH_Dvm:
  case TPC::LD_TNSR_HIGH_Dmp:
  case TPC::LD_TNSR_HIGH_Dmm:

  case TPC::LD_TNSR_HIGH_PDvp:
  case TPC::LD_TNSR_HIGH_PDvm:
  case TPC::LD_TNSR_HIGH_PDmp:
  case TPC::LD_TNSR_HIGH_PDmm:

  case TPC::LD_TNSR_CNVRT_Dvp:
  case TPC::LD_TNSR_CNVRT_Dvm:

  case TPC::LD_TNSR_CNVRT_PDvp:
  case TPC::LD_TNSR_CNVRT_PDvm:

  case TPC::LD_TNSR_ST_Dvp:
  case TPC::LD_TNSR_ST_Dvm:
  case TPC::LD_TNSR_ST_Dmp:
  case TPC::LD_TNSR_ST_Dmm:

  case TPC::LD_TNSR_ST_PDvp:
  case TPC::LD_TNSR_ST_PDvm:
  case TPC::LD_TNSR_ST_PDmp:
  case TPC::LD_TNSR_ST_PDmm:

  case TPC::LD_TNSR_LOW_ST_Dvp:
  case TPC::LD_TNSR_LOW_ST_Dvm:
  case TPC::LD_TNSR_LOW_ST_Dmp:
  case TPC::LD_TNSR_LOW_ST_Dmm:

  case TPC::LD_TNSR_LOW_ST_PDvp:
  case TPC::LD_TNSR_LOW_ST_PDvm:
  case TPC::LD_TNSR_LOW_ST_PDmp:
  case TPC::LD_TNSR_LOW_ST_PDmm:

  case TPC::LD_TNSR_HIGH_ST_Dvp:
  case TPC::LD_TNSR_HIGH_ST_Dvm:
  case TPC::LD_TNSR_HIGH_ST_Dmp:
  case TPC::LD_TNSR_HIGH_ST_Dmm:

  case TPC::LD_TNSR_HIGH_ST_PDvp:
  case TPC::LD_TNSR_HIGH_ST_PDvm:
  case TPC::LD_TNSR_HIGH_ST_PDmp:
  case TPC::LD_TNSR_HIGH_ST_PDmm:

  case TPC::LD_TNSR_CNVRT_D_ST_vp:
  case TPC::LD_TNSR_CNVRT_D_ST_vm:
  case TPC::LD_TNSR_CNVRT_PD_ST_vp:
  case TPC::LD_TNSR_CNVRT_PD_ST_vm:
    return false;
  }
  return true;
}

static MachineOperand &getSwitches(MachineInstr &MI) {
  unsigned SwitchOpNum;
  const MCInstrDesc &D = MI.getDesc();
  if (MI.getOpcode() == TPC::LD_GIap ||
      MI.getOpcode() == TPC::LD_GIwap ||
      MI.getOpcode() == TPC::LD_Gg4Iap ||
      MI.getOpcode() == TPC::LD_Gg4Iwap) {
    SwitchOpNum = N_Switches_in_LD_GI;
  } else if (TPCII::isStoreInst(D) && TPCII::getSlotOpCode(D) == TPCII::ST_G) {
    SwitchOpNum = N_Switches_in_ST_G;
  } else if (TPCII::isLdTnsr(D, true)) {
    SwitchOpNum = N_Switches_in_LD_TNSR;
  } else if (TPCII::isStTnsr(D)) {
    SwitchOpNum = N_Switches_in_ST_TNSR;
  } else if (TPCII::isLoadInst(D) && TPCII::getSlotOpCode(D) == TPCII::PREFETCH) {
    SwitchOpNum = N_Switches_in_PREFETCH;
  } else if (TPCII::isLoadInst(D) && TPCII::getSlotOpCode(D) == TPCII::LD_G) {
    SwitchOpNum = N_Switches_in_LD_G;
  } else if (TPCII::isGenAddr(D)) {
    SwitchOpNum = N_Switches_in_GEN_ADDR;
  } else {
    llvm_unreachable("Unsupported instruction");
  }
  MachineOperand &Result = MI.getOperand(SwitchOpNum);
  assert(Result.isImm());
  return Result;
}

// Check if the provided instruction is an increment of index register, suitable
// for auto-increment pattern.
static bool isValidIndexIncValue(MachineInstr *Increment, int &IncVal) {
  if (Increment->getOpcode() != TPC::ADDiIp)
    return false;
  int64_t IncValOp = Increment->getOperand(N_Imm_in_ADD).getImm();
  switch (IncValOp) {
  case 1:
  case 2:
  case 4:
  case 8:
    break;
  default:
    return false;
  }
  int64_t DimMask = Increment->getOperand(N_Mask_in_ADD).getImm();
  if (DimMask != 0b00001)
    return false;
  IncVal = IncValOp;
  return true;
}

// Get element type encoded in GEN_ADDR instruction.
static unsigned getDataSize(MachineInstr &GenAddr) {
  assert(TPCII::isGenAddr(GenAddr.getDesc()));
  MachineOperand &SwitchesMO = GenAddr.getOperand(N_Switches_in_GEN_ADDR);
  assert(SwitchesMO.isImm());
  unsigned Switches = SwitchesMO.getImm();
  assert(Switches & TPCII::SW_DT_OVERRIDE);
  Switches &= TPCII::SW_TYPE;
  switch (Switches) {
  case TPCII::SW_DT_INT64:
    return 8;
  case TPCII::SW_DT_FP32:
  case TPCII::SW_DT_INT32:
  case TPCII::SW_DT_UINT32:
    return 4;
  case TPCII::SW_DT_BF16:
  case TPCII::SW_DT_FP16:
  case TPCII::SW_DT_INT16:
  case TPCII::SW_DT_UINT16:
    return 2;
  case TPCII::SW_DT_INT8:
  case TPCII::SW_DT_UINT8:
  case TPCII::SW_DT_FP8_152:
  case TPCII::SW_DT_FP8_143:
    return 1;
  }
  llvm_unreachable("Invalid gen_addr data type");
}

static bool isBeroreThan(MachineInstr &Instr, MachineInstr &Ref) {
  MachineBasicBlock *BB = Instr.getParent();
  if (!BB || Ref.getParent() != BB)
    return false;
  
  for (auto I = BB->begin(), E = BB->end(); I != E; ++I) {
    MachineInstr &MI = *I;
    if (&MI == &Instr)
      return true;
    if (&MI == &Ref)
      break;
  }
  return false;
}

static MachineInstr *getMergeableIndexIncrement(MachineInstr *TensorOp,
                                                MachineRegisterInfo *MRI,
                                                int &IncVal,
                                                unsigned &DimMask) {
  const unsigned IRFRegNo = TPCII::isStTnsr(TensorOp->getDesc()) ?
      N_Index_in_ST_TNSR : N_Index_in_LD_TNSR;
  MachineOperand &Coords = TensorOp->getOperand(IRFRegNo);
  Register IRF = Coords.getReg();
  auto UsePtr = MRI->use_instr_nodbg_begin(IRF);
  auto EndPtr = MRI->use_instr_nodbg_end();
  for (; UsePtr != EndPtr; ++UsePtr) {
    MachineInstr &User = *UsePtr;
    if (&User == TensorOp)
      break;
  }
  if (UsePtr == EndPtr)
    return nullptr;
  MachineInstr *Increment = nullptr;
  for (++UsePtr; UsePtr != EndPtr; ++UsePtr) {
    MachineInstr &User = *UsePtr;
    if (User.getOpcode() == TPC::ADDiIp) {
      if (Increment)
        return nullptr;
      Increment = &User;
    } else {
      return nullptr;
    }
  }
  if (!Increment)
    return nullptr;

  int64_t DimMaskOp = Increment->getOperand(N_Mask_in_ADD).getImm();
  switch (DimMaskOp) {
  case 0b00001:
  case 0b00010:
  case 0b00100:
  case 0b01000:
  case 0b10000:
    break;
  default:
    return nullptr;
  }

  int64_t IncValOp = Increment->getOperand(N_Imm_in_ADD).getImm();
  if (IncValOp <= 0)
    return nullptr;
  IncVal = IncValOp;
  DimMask = DimMaskOp;
  return Increment;
}

// Creates definition operand in the instruction that supports auto-incrementing
// ADRF.
static void addIncrementedADRFDef(MachineInstr &MI,
                                  unsigned ADRFOpNo,
                                  MachineOperand &OldIncrementedDef,
                                  MachineRegisterInfo *MRI) {
  unsigned IncrementedADRF = MRI->createVirtualRegister(&TPC::ADRFRegClass);
  MachineOperand NewADRF = MachineOperand::CreateReg(IncrementedADRF, /*isDef*/ true, /*isImp*/ true);
  MI.addOperand(NewADRF);
  MI.tieOperands(MI.getNumOperands() - 1, ADRFOpNo);
  MRI->replaceRegWith(OldIncrementedDef.getReg(), IncrementedADRF);
}

bool TPCIncMerger::isFoldableLoadTensor(MachineInstr *MI,
                                        FoldableOperation &Operation) {
  if (is_LD_TNSR_with_auto_inc_form(MI)) {
    int IncVal;
    unsigned DimMask;
    if (MachineInstr *IncMI = getMergeableIndexIncrement(MI, MRI, IncVal, DimMask)) {
      Operation.Kind = MergeKind::LoadTensor;
      Operation.Source.push_back(MI);
      Operation.Source.push_back(IncMI);
      Operation.BB = MI->getParent();
      Operation.IncVal = IncVal;
      Operation.DimMask = DimMask;
      return true;
    }
  }
  return false;
}

bool TPCIncMerger::isFoldableSaveTensor(MachineInstr *MI,
                                        FoldableOperation &Operation) {
  if (is_ST_TNSR_with_auto_inc_form(MI)) {
    int IncVal;
    unsigned DimMask;
    if (MachineInstr *IncMI = getMergeableIndexIncrement(MI, MRI, IncVal, DimMask)) {
      Operation.Kind = MergeKind::LoadTensor;
      Operation.Source.push_back(MI);
      Operation.Source.push_back(IncMI);
      Operation.BB = MI->getParent();
      Operation.IncVal = IncVal;
      Operation.DimMask = DimMask;
      return true;
    }
  }
  return false;
}

// Returns number of the instruction operand that corresponds to global address.
// The kind of instruction is specified by 'K'.
static unsigned getAddressOperand(MergeKind K) {
  switch (K) {
  case MergeKind::Prefetch:
    return N_Addr_in_PREFETCH;
  case MergeKind::LoadGlobal:
    return N_Addr_in_LD_G;
  case MergeKind::LoadGlobalIRF:
    return N_Addr_in_LD_GI;
  case MergeKind::StoreGlobal:
    return N_Addr_in_ST_G;
  default:
    break;
  }
  llvm_unreachable("No address operand for this instruction");
}

// Checks if the given instruction can be converted into auto-increment form.
//
// An instruction can be folded to auto-increment form if it participates in
// a pattern like:
//
//     addr = GEN_ADDR tensor, index
//     AccessMI addr
//     index.1 = ADDiIp index, N
//     addr.1 = GEN_ADDR tensor, index.1
//
// If the instruction can be converted to auto-increment form, the function
// returns 'true' and 'Operation' is initialized with proper information about
// the recognized pattern.
//
bool TPCIncMerger::matchAutoIncPattern(MachineInstr *AccessMI,
                                       FoldableOperation &Operation) {
  MachineBasicBlock *BB = AccessMI->getParent();

  // GEN_ADDR for the AccessMI.
  //
  // It must be consumed in one instruction only, which is AccessMI.
  const unsigned AddressOperandNo = getAddressOperand(Operation.Kind);
  Register Address = AccessMI->getOperand(AddressOperandNo).getReg();
  if (!MRI->hasOneNonDBGUse(Address)) {
    LLVM_DEBUG(dbgs() << "Rejected: Address has more than one use\n";);
    return false;
  }
  MachineOperand *DefMO = MRI->getOneDef(Address);
  MachineInstr *GenAddr1 = DefMO->getParent();
  if (!TPCII::isGenAddr(GenAddr1->getDesc())) {
    LLVM_DEBUG(dbgs() << "Rejected: Address comes not from GEN_ADDR\n";);
    return false;
  }
  if (GenAddr1->getParent() != BB) {
     LLVM_DEBUG(dbgs() << "Rejected: GEN_ADDR in other block\n";);
   return false;
  }

  // ADD that increments index register.
  MachineInstr *IndexInc = nullptr;
  MachineInstr *GenAddr2 = nullptr;
  int IncVal = 0;
  Register Index = GenAddr1->getOperand(N_Index_in_GEN_ADDR).getReg();
  // Scan all uses of the index register to find proper ADD instruction.
  auto UsePtr = MRI->use_instr_nodbg_begin(Index);
  auto EndPtr = MRI->use_instr_nodbg_end();
  for (; UsePtr != EndPtr; ++UsePtr) {
    MachineInstr &User = *UsePtr;
    LLVM_DEBUG(dbgs() << "Analysing IRF users: "; User.dump());
    if (User.getParent() != BB)
      continue;
    if (&User == GenAddr1)
      continue;
    if (isValidIndexIncValue(&User, IncVal)) {
      // Candidate ADD instruction has been found. It however increments index
      // register, but we need pointer increment. To make this convertion, we
      // need data element size. It can be obtained either from GEN_ADDR or
      // from AccessMI.
      MachineOperand &SwitchesOp = getSwitches(*GenAddr1);
      unsigned SwitchVal = SwitchesOp.getImm();
      unsigned DataSize = 0;
      if (SwitchVal & TPCII::SW_DT_OVERRIDE) {
        DataSize = getDataSize(*GenAddr1);
      } else {
        if (!DeduceIncrement)
          return false;
        if (AccessMI->memoperands_empty() || AccessMI->memoperands().size() != 1)
          return false;
        MachineMemOperand *MO = AccessMI->memoperands().front();
        assert(MO);
        DataSize = MO->getSize();
      }

      // Calculate and check pointer increment.
      IncVal *= DataSize;
      switch (IncVal) {
      case 1:
      case 2:
      case 4:
      case 8:
        break;
      default:
        return false;
      }

      // Look for GEN_ADDR after AccessMI.
      Register Index2 = User.getOperand(0).getReg();
      for (auto U = MRI->use_instr_nodbg_begin(Index2),
                E = MRI->use_instr_nodbg_end(); U != E; ++U) {
        MachineInstr &Index2User = *U;
        if (Index2User.getParent() != BB)
          continue;
        if (TPCII::isGenAddr(Index2User.getDesc())) {
          MachineOperand &Tensor1MO = GenAddr1->getOperand(1);
          MachineOperand &Tensor2MO = Index2User.getOperand(1);
          if (Tensor2MO.isImm()) {
            if (!Tensor1MO.isImm())
              continue;
            if (Tensor1MO.getImm() != Tensor2MO.getImm())
              continue;
          } else {
            if (!Tensor1MO.isReg())
              continue;
            if (Tensor1MO.getReg() != Tensor2MO.getReg())
              continue;
          }
          IndexInc = &User;
          GenAddr2 = &Index2User;
          break;
        }
      }
    }
  }
  if (!IndexInc)
    return false;

  // All users of GenAddr2 must occur after AccessMI.
  Register Addr2 = GenAddr2->getOperand(0).getReg();
  auto Addr2UsePtr = MRI->use_instr_nodbg_begin(Addr2);
  auto Addr2EndPtr = MRI->use_instr_nodbg_end();
  if (Addr2UsePtr != Addr2EndPtr) {
    MachineInstr &FirstUser = *Addr2UsePtr;
    if (isBeroreThan(FirstUser, *AccessMI))
      return false;
  }

  Operation.Source.push_back(AccessMI);
  Operation.Source.push_back(GenAddr1);
  Operation.Source.push_back(IndexInc);
  Operation.Source.push_back(GenAddr2);
  Operation.BB = BB;
  Operation.IncVal = IncVal;
  Operation.DimMask = 0b00001;
  return true;
}

bool TPCIncMerger::isFoldablePrefetch(MachineInstr *MI,
                                      FoldableOperation &Operation) {
  // Try to find instruction pattern:
  //
  // addr = GEN_ADDR tensor, index
  // PREFETCH addr
  // index.1 = ADDiIp index, N
  // addr.1 = GEN_ADDR tensor, index.1

  // PREFETCH
  const MCInstrDesc &D = MI->getDesc();
  if (!TPCII::isLoadInst(D))
    return false;
  unsigned Opcode = TPCII::getSlotOpCode(D);
  if (Opcode != TPCII::PREFETCH)
    return false;
  MachineOperand &SwitchesMO = getSwitches(*MI);
  unsigned Switches = SwitchesMO.getImm();
  if (Switches & TPCII::SW_INC_VAL)
    return false;

  LLVM_DEBUG(dbgs() << "Analysing: "; MI->dump(););

  Operation.Kind = MergeKind::Prefetch;
  return matchAutoIncPattern(MI, Operation);
}

bool TPCIncMerger::isFoldableLoadGlobal(MachineInstr *MI,
                                        FoldableOperation &Operation) {
  // Try to find instruction pattern:
  //
  // addr = GEN_ADDR tensor, index
  // v = LD_G addr
  // index.1 = ADDiIp index, N
  // addr.1 = GEN_ADDR tensor, index.1

  // LD_G
  const MCInstrDesc &D = MI->getDesc();
  if (!TPCII::isLoadInst(D))
    return false;
  unsigned Opcode = TPCII::getSlotOpCode(D);
  if (Opcode != TPCII::LD_G)
    return false;
  if (MI->getDesc().getNumDefs() > 1)
    return false;

  MachineOperand &SwitchesMO = getSwitches(*MI);
  unsigned Switches = SwitchesMO.getImm();
  if (ST->hasGrecoISA()) {
    if (Switches & TPCII::SW_AUTO_INC_G3)
      return false;
  } else {
    if (Switches & TPCII::SW_INC_VAL)
      return false;
  }

  LLVM_DEBUG(dbgs() << "Analysing: "; MI->dump(););

  if (MRI->getRegClass(MI->getOperand(0).getReg())->hasSubClassEq(&TPC::IRFRegClass)) {
    Operation.Kind = MergeKind::LoadGlobalIRF;
  } else {
    Operation.Kind = MergeKind::LoadGlobal;
  }

  return matchAutoIncPattern(MI, Operation);
}

bool TPCIncMerger::isFoldableStoreGlobal(MachineInstr *MI,
                                         FoldableOperation &Operation) {
  // Try to find instruction pattern:
  //
  // addr = GEN_ADDR tensor, index
  // ST_G addr, v
  // index.1 = ADDiIp index, N
  // addr.1 = GEN_ADDR tensor, index.1

  // ST_G
  const MCInstrDesc &D = MI->getDesc();
  if (!TPCII::isStoreInst(D))
    return false;
  unsigned Opcode = TPCII::getSlotOpCode(D);
  if (Opcode != TPCII::ST_G)
    return false;
  if (MI->getDesc().getNumDefs() > 0)
    return false;

  MachineOperand &SwitchesMO = getSwitches(*MI);
  unsigned Switches = SwitchesMO.getImm();
  if (Switches & TPCII::SW_INC_VAL)
    return false;

  LLVM_DEBUG(dbgs() << "Analysing: "; MI->dump(););

  Operation.Kind = MergeKind::StoreGlobal;
  return matchAutoIncPattern(MI, Operation);
}

static MachineLoop *findInnermostLoop(MachineLoop *L) {
  if (L->begin() == L->end())
    return L;

  MachineLoop::iterator I = L->begin(), E = L->end();
  MachineLoop *Innermost = *I++;
  for (; I != E; ++I) {
    MachineLoop *InnerLoop = findInnermostLoop(*I);
    if (InnerLoop->getLoopDepth() > Innermost->getLoopDepth())
      Innermost = InnerLoop;
  }
  return Innermost;
}

static void setLdIncValueReg(MachineBasicBlock *MBB, int Delta, const TPCInstrInfo *TII) {
  BuildMI(*MBB, MBB->getFirstTerminator(), DebugLoc(), TII->get(TPC::MOV_ld_hsip), Register(TPC::INC_LD_DIM_REG))
    .addImm(Delta)
    .addImm(0)
    .addReg(TPC::IMPLICIT_DEF)
    .addReg(TPC::SPRF_TRUE)
    .addImm(0);
}

static void setStIncValueReg(MachineBasicBlock *MBB, int Delta, const TPCInstrInfo *TII) {
  BuildMI(*MBB, MBB->getFirstTerminator(), DebugLoc(), TII->get(TPC::MOV_ld_hsip), Register(TPC::INC_ST_DIM_REG))
    .addImm(Delta)
    .addImm(0)
    .addReg(TPC::IMPLICIT_DEF)
    .addReg(TPC::SPRF_TRUE)
    .addImm(0);
}

bool TPCIncMerger::foldIndexIncrementInScalarOp(const FoldableOperation &Op) {
  if (Op.Kind == MergeKind::Prefetch) {
    MachineInstr &Prefetch = *Op.Source[0];
    MachineInstr &GenAddr1 = *Op.Source[1];
    MachineInstr &Increment = *Op.Source[2];
    MachineInstr &GenAddr2 = *Op.Source[3];
    assert(TPCII::isLoadInst(Prefetch.getDesc()));
    assert(TPCII::getSlotOpCode(Prefetch.getDesc()) == TPCII::PREFETCH);
    assert(TPCII::isGenAddr(GenAddr1.getDesc()));
    assert(Increment.getOpcode() == TPC::ADDiIp);
    assert(TPCII::isGenAddr(GenAddr2.getDesc()));
    MachineOperand &SwitchesOp = getSwitches(Prefetch);
    unsigned Switches = SwitchesOp.getImm();
    Switches &= ~TPCII::SW_INC_VAL;
    int IncVal = Op.IncVal;
    int IncSwitches = 0;
    switch (IncVal) {
    case 1: IncSwitches = TPCII::SW_INC_1; break;
    case 2: IncSwitches = TPCII::SW_INC_2; break;
    case 4: IncSwitches = TPCII::SW_INC_4; break;
    case 8: IncSwitches = TPCII::SW_INC_8; break;
    default:
      llvm_unreachable("Unexpected increment");
    }
    Switches |= IncSwitches;
    SwitchesOp.setImm(Switches);
    addIncrementedADRFDef(Prefetch, N_Addr_in_PREFETCH, GenAddr2.getOperand(0), MRI);
    GenAddr2.eraseFromParentAndMarkDBGValuesForRemoval();
    Increment.eraseFromParentAndMarkDBGValuesForRemoval();
    return true;
  }

  if (Op.Kind == MergeKind::LoadGlobal || Op.Kind == MergeKind::LoadGlobalIRF) {
    MachineInstr &Load = *Op.Source[0];
    MachineInstr &GenAddr1 = *Op.Source[1];
    MachineInstr &Increment = *Op.Source[2];
    MachineInstr &GenAddr2 = *Op.Source[3];
    assert(TPCII::isLoadInst(Load.getDesc()));
    // FIXME: assert(Load.memoperands().size() == 1);
    assert(TPCII::getSlotOpCode(Load.getDesc()) == TPCII::LD_G);
    assert(TPCII::isGenAddr(GenAddr1.getDesc()));
    assert(Increment.getOpcode() == TPC::ADDiIp);
    assert(TPCII::isGenAddr(GenAddr2.getDesc()));

    MachineOperand &SwitchesOp = getSwitches(Load);
    unsigned Switches = SwitchesOp.getImm();
    unsigned FlagMask = 0;
    if (ST->hasGrecoISA())
      FlagMask = TPCII::SW_AUTO_INC_G3 | TPCII::SW_INC_VAL_G3;
    else if (ST->hasGaudi2ISA())
      FlagMask = TPCII::SW_INC_VAL;
    else if (ST->hasDoron1())
      FlagMask = TPCII::SW_INC_VAL;
    else
      return false;
    Switches &= ~FlagMask;

    int IncVal = Op.IncVal;

    int IncSwitches = 0;
    if (ST->hasGrecoISA()) {
      switch (IncVal) {
      case 1: IncSwitches = TPCII::SW_INC_1_G3; break;
      case 2: IncSwitches = TPCII::SW_INC_2_G3; break;
      case 4: IncSwitches = TPCII::SW_INC_4_G3; break;
      case 8: IncSwitches = TPCII::SW_INC_8_G3; break;
      default:
        return false;
      }
      IncSwitches |= TPCII::SW_AUTO_INC_G3;
    } else {
      switch (IncVal) {
      case 1: IncSwitches = TPCII::SW_INC_1; break;
      case 2: IncSwitches = TPCII::SW_INC_2; break;
      case 4: IncSwitches = TPCII::SW_INC_4; break;
      case 8: IncSwitches = TPCII::SW_INC_8; break;
      default:
        return false;
      }
    }
    Switches |= IncSwitches;
    SwitchesOp.setImm(Switches);
    unsigned ADRFOpNo;
    if (Op.Kind == MergeKind::LoadGlobal)
      ADRFOpNo = N_Addr_in_LD_G;
    else
      ADRFOpNo = N_Addr_in_LD_GI;
    addIncrementedADRFDef(Load, ADRFOpNo, GenAddr2.getOperand(0), MRI);
    GenAddr2.eraseFromParentAndMarkDBGValuesForRemoval();
    Increment.eraseFromParentAndMarkDBGValuesForRemoval();
    return true;
  }

  if (Op.Kind == MergeKind::StoreGlobal) {
    MachineInstr &Store = *Op.Source[0];
    MachineInstr &GenAddr1 = *Op.Source[1];
    MachineInstr &Increment = *Op.Source[2];
    MachineInstr &GenAddr2 = *Op.Source[3];
    assert(TPCII::isStoreInst(Store.getDesc()));
    assert(Store.memoperands().size() == 1);
    assert(TPCII::getSlotOpCode(Store.getDesc()) == TPCII::ST_G);
    assert(TPCII::isGenAddr(GenAddr1.getDesc()));
    assert(Increment.getOpcode() == TPC::ADDiIp);
    assert(TPCII::isGenAddr(GenAddr2.getDesc()));

    MachineOperand &SwitchesOp = getSwitches(Store);
    unsigned Switches = SwitchesOp.getImm();
    unsigned FlagMask = 0;
    if (ST->hasGaudi2ISA())
      FlagMask = TPCII::SW_INC_VAL;
    else
      return false;
    Switches &= ~FlagMask;

    int IncVal = Op.IncVal;
    int IncSwitches = 0;
    switch (IncVal) {
    case 1: IncSwitches = TPCII::SW_INC_1; break;
    case 2: IncSwitches = TPCII::SW_INC_2; break;
    case 4: IncSwitches = TPCII::SW_INC_4; break;
    case 8: IncSwitches = TPCII::SW_INC_8; break;
    default:
      return false;
    }
    Switches |= IncSwitches;
    SwitchesOp.setImm(Switches);
    addIncrementedADRFDef(Store, N_Addr_in_ST_G, GenAddr2.getOperand(0), MRI);
    GenAddr2.eraseFromParentAndMarkDBGValuesForRemoval();
    Increment.eraseFromParentAndMarkDBGValuesForRemoval();
    return true;
  }

  return false;
}

static bool foldIndexIncrement(const FoldableOperation &Op,
                               MachineRegisterInfo *MRI,
                               const TPCSubtarget &ST) {
  if (Op.Kind == MergeKind::LoadTensor || Op.Kind == MergeKind::StoreTensor) {
    MachineInstr &TensorOp = *Op.Source[0];
    MachineInstr &Increment = *Op.Source[1];

    MachineOperand &Coords = TensorOp.getOperand(1);
    Register IRF = Coords.getReg();
    MachineOperand &Switches = getSwitches(TensorOp);
    int64_t SwitchVal = Switches.getImm();
    SwitchVal &= ~TPCII::SW_AUTO_INC_DIM;
    switch (Op.DimMask) {
    case 0x01:
      SwitchVal |= TPCII::SW_INC_DIM0;
      break;
    case 0x02:
      SwitchVal |= TPCII::SW_INC_DIM1;
      break;
    case 0x04:
      SwitchVal |= TPCII::SW_INC_DIM2;
      break;
    case 0x08:
      SwitchVal |= TPCII::SW_INC_DIM3;
      break;
    case 0x10:
      SwitchVal |= TPCII::SW_INC_DIM4;
      break;
    default:
      return false;
    }
    Switches.setImm(SwitchVal);
    unsigned IncrementedIRF = MRI->createVirtualRegister(&TPC::IRFRegClass);
    MachineOperand NewIRF = MachineOperand::CreateReg(IncrementedIRF, /*isDef*/ true, /*isImp*/ true);
    TensorOp.addOperand(NewIRF);
    if (Op.Kind == MergeKind::StoreTensor) {
      TensorOp.tieOperands(TensorOp.getNumOperands() - 1, N_Index_in_ST_TNSR);
      TensorOp.addOperand(MachineOperand::CreateReg(TPC::INC_ST_DIM_REG, /*isDef*/ false, /*isImp*/ true));
    } else {
      TensorOp.tieOperands(TensorOp.getNumOperands() - 1, N_Index_in_LD_TNSR);
      TensorOp.addOperand(MachineOperand::CreateReg(TPC::INC_LD_DIM_REG, /*isDef*/ false, /*isImp*/ true));
    }
    MRI->replaceRegWith(Increment.getOperand(0).getReg(), IncrementedIRF);
    Coords.setIsKill();
    Increment.eraseFromParentAndMarkDBGValuesForRemoval();
    return true;
  }

  return false;
}

bool TPCIncMerger::runOnMachineFunction(MachineFunction &Func) {
  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnableIncMerger)
    return false;

  MF = &Func;
  MLI = &getAnalysis<MachineLoopInfo>();
  MDT = &getAnalysis<MachineDominatorTree>();
  MRI = &Func.getRegInfo();
  ST = &MF->getSubtarget<TPCSubtarget>();
  TII = ST->getInstrInfo();

  int NumReplaced = 0;

  //--- Folding of scalar global memory access.
  //
  // These operations are peephole transformations.

  for (auto &BB : Func) {
    bool FoldingMade;
    do {
      FoldingMade = false;
      for (auto IPtr = BB.instr_rbegin(), E = BB.instr_rend(); IPtr != E;) {
        MachineInstr &I = *IPtr;
        ++IPtr;
        FoldableOperation Op;
        if ((ST->hasDoron1() && isFoldablePrefetch(&I, Op)) ||
            (ST->hasGen4Plus() && isFoldableStoreGlobal(&I, Op)) ||
            (ST->hasGen3Plus() && isFoldableLoadGlobal(&I, Op))) {
          LLVM_DEBUG(dbgs() << "Folding\n";);
          if (foldIndexIncrementInScalarOp(Op)) {
            ++NumReplaced;
            FoldingMade = true;
            break;
          }
        }
      }
    } while (FoldingMade);
  }

  //--- Folding of tensor global memory access.
  //
  // These operations require code insertions to various blocks.

  // Collect all instances where folding is possible. Do not fold them however.
  SmallVector<FoldableOperation, 16> FoldableOperations;
  for (auto &BB : Func) {
    for (auto IPtr = BB.begin(), E = BB.end(); IPtr != E;) {
      MachineInstr &I = *IPtr;
      ++IPtr;
      LLVM_DEBUG(
        dbgs() << "Analysing: ";
        I.dump();
      );
      FoldableOperation Op;
      if ((ST->hasDoron1() && isFoldableLoadTensor(&I, Op)) ||
          (ST->hasDoron1() && isFoldableSaveTensor(&I, Op))) {
        Op.LoopDepth = MLI->getLoopDepth(Op.BB);
        if (Op.LoopDepth != 0)
          FoldableOperations.push_back(Op);
      }
    }
  }
  if (FoldableOperations.empty())
    return false;
  LLVM_DEBUG(
    dbgs() << "Foldable operations:\n";
    for (const FoldableOperation &FO : FoldableOperations) {
      dbgs() << "  " << FO.Kind << '\n';
      for (auto *I : FO.Source) {
        dbgs() << "  ";
        I->dump();
      }
    }
  );

  // Prepare mapping loop->foldable_operations.
  std::map<MachineLoop *, std::vector<FoldableOperation>> LdTensorOpsByLoops;
  std::map<MachineLoop *, std::vector<FoldableOperation>> StTensorOpsByLoops;
  for (FoldableOperation &FO : FoldableOperations) {
    MachineLoop *L = MLI->getLoopFor(FO.BB);
    if (FO.Kind == MergeKind::LoadTensor)
      LdTensorOpsByLoops[L].push_back(FO);
    else if (FO.Kind == MergeKind::StoreTensor)
      StTensorOpsByLoops[L].push_back(FO);
  }

  // Set increment value registers. Each top-level loop has its own value and
  // that value is determined by the first operation in the innermost loop in
  // that loop.
  int CurrentLdIncValue = 0;
  int CurrentStIncValue = 0;
  std::map<MachineLoop *, int> LdIncrementPerTopLevelLoop;
  std::map<MachineLoop *, int> StIncrementPerTopLevelLoop;
  
  for (auto I = MLI->begin(), E = MLI->end(); I != E; ++I) {
    MachineLoop *L = findInnermostLoop(*I);

    std::vector<FoldableOperation> &TensorLdOps = LdTensorOpsByLoops[L];
    if (!TensorLdOps.empty()) {
      int IncVal = TensorLdOps.front().IncVal;
      LdIncrementPerTopLevelLoop[*I] = IncVal;
      if (IncVal != CurrentLdIncValue) {
        CurrentLdIncValue = IncVal;
        MachineBasicBlock *Header = (*I)->getHeader();
        MachineBasicBlock *Dom = MDT->getNode(Header)->getIDom()->getBlock();
        setLdIncValueReg(Dom, CurrentLdIncValue, TII);
      }
    }
    std::vector<FoldableOperation> &TensorStOps = StTensorOpsByLoops[L];
    if (!TensorStOps.empty()) {
      int IncVal = TensorStOps.front().IncVal;
      StIncrementPerTopLevelLoop[*I] = IncVal;
      if (IncVal != CurrentStIncValue) {
        CurrentStIncValue = IncVal;
        MachineBasicBlock *Header = (*I)->getHeader();
        MachineBasicBlock *Dom = MDT->getNode(Header)->getIDom()->getBlock();
        setStIncValueReg(Dom, CurrentStIncValue, TII);
      }
    }
  }

  // Fold operations with increments, if possible.
  for (FoldableOperation &FO : FoldableOperations) {
    if (FO.Kind == MergeKind::LoadTensor || FO.Kind == MergeKind::StoreTensor) {
      MachineLoop *TopLevelLoop = MLI->getLoopFor(FO.BB);
      while (MachineLoop *ParentLoop = TopLevelLoop->getParentLoop())
        TopLevelLoop = ParentLoop;
      int IncVal = 0;
      if (FO.Kind == MergeKind::LoadTensor) {
        assert(LdIncrementPerTopLevelLoop.find(TopLevelLoop) != LdIncrementPerTopLevelLoop.end());
        IncVal = LdIncrementPerTopLevelLoop[TopLevelLoop];
      } else {
        assert(StIncrementPerTopLevelLoop.find(TopLevelLoop) != StIncrementPerTopLevelLoop.end());
        IncVal = StIncrementPerTopLevelLoop[TopLevelLoop];
      }
      if (FO.IncVal != IncVal)
        continue;
      if (foldIndexIncrement(FO, MRI, *ST))
        ++NumReplaced;
    }
  }

  return NumReplaced != 0;
}

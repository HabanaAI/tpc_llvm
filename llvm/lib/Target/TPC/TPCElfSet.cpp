//===- TPCELFSet.cpp--------Set ELF's TPC Program Header------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
// author: Michael Zuckerman
//         mzuckerman@habana.ai
//===----------------------------------------------------------------------===//
// 1) This pass adds metadata to the ELF under the name of TPC_METADATA.
// 2) The layout is in TPCMetadataSection.cpp.
// 3) This pass searches for lookup instruction if exist set
//    report.specialFunction to true.
// 4) This pass searches for printf metadata if exist set printf to true.
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCTargetMachine.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TPCClangInfo.h"
#include "llvm/Target/TPCMetadataSection.h"
#include <algorithm>
#include <iterator>
#include <limits>

#define DEBUG_TYPE "TPCElfSpec"

using namespace llvm;
using namespace std;
namespace llvm {
FunctionPass *createTPCElfSpecSet();
void initializeTPCElfSpecSetPass(PassRegistry &);
} // namespace llvm

static const char PassDescription[] = "Set the ELF program Header";
static const char PassName[] = "TPCElfSpecSet";
static cl::opt<bool> EnableElfSpec("tpc-elf-spec", cl::init(true), cl::Hidden);
static cl::opt<bool> LockUnlockMismatchError("tpc-lock-mismatch-error",
  cl::Hidden,
  cl::init(true));

namespace {
class TPCElfSpecSet : public MachineFunctionPass {
#define numberOfLockRegister 32
  typedef enum {TT_INPUT, TT_OUTPUT} t_tensorType;
  typedef enum { MM_MAX, MM_MIN } t_minMax;
  bool HasRMWStore[TPCNumTensors] = {false};
  bool AllTrueRMW = false;
  std::map<int, t_tensorType> TensorMap;
  typedef enum { LU_LOCK, LU_UNLOCK } t_LockUnlock;
  std::pair<bool, bool> lockUnlock[numberOfLockRegister + 1];
  TPCMetadataSection TPCMetadata;
  /*!
   *
   * @param BMI: machine instruction , contains one instruction from the VLIW
   * The function doesn't return rather set the TensorMap member.
   */
  void mapInputOutputTensor(const MachineInstr &BMI);
  /*!
   *
   * @param BMI: machine instruction , contains one instruction from the VLIW
   * The function doesn't return rather set the TPCMetadata member.
   */
  void detectMMIO(const MachineInstr &BMI);
  /*!
   *
   * @param tensor tensor type for query the data.
   * @param minMax check minimum tensor id or max
   * @return return pair of min/max id and tensor type.
   */
  std::pair<int, t_tensorType>
  findMaxMinTensorNumber(t_tensorType tensor,
                              t_minMax minMax = MM_MAX);
  /*!
   *
   * \breaf searchInstruction: The function search for instruction and set report
   * \return void
   */
  void searchInstruction(MachineFunction &Func);
  /*!
   *
   * \breaf searchPrintF: The function search for printf in the metadata
   * section. If exist printf is set to true else printf is set to false
   * \param Func --> MachineFunction as a bundle
   * \return bool value
   */
  void searchPrintF(MachineFunction &Func);

  void CalculateTPCMetadata(const MachineFunction &Func);
  /*!
   * \breaf writeResult the function write the result of the report to a
   * new section in the ELF. The name of the section is TPC_METADATA.
   * \param Func --> MachineFunction as a bundle
   */
  void createElfSection(MachineFunction &Func);

  int getLockUnlockIndex(int address);
  void setLockUnlockMember(t_LockUnlock SW, int idx);
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTreeWrapperPass>();
    AU.addPreserved<MachineDominatorTreeWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  void setTensorMap(MachineFunction &Func);

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCElfSpecSet() : MachineFunctionPass(ID) {
    initializeTPCElfSpecSetPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // namespace

/// Marks \p Tensor in \p Mask. A tensor ID in a register can name any tensor.
static void setTensorBits(bool (&Mask)[TPCNumTensors],
                          const MachineOperand &Tensor) {
  if (Tensor.isImm())
    Mask[Tensor.getImm()] = true;
  else
    std::fill(std::begin(Mask), std::end(Mask), true);
}

/// Whether the st_tnsr \p MI may write only part of a tensor element. This
/// matches what Habana's compiler reports for each form of the instruction.
static bool isPartialStore(const MachineInstr &MI) {
  const unsigned Switches = getSwitches(MI);
  switch (TPCII::getSlotOpCode(MI.getDesc())) {
  case TPCII::ST_TNSR:
    return (Switches & TPCII::SW_PARTIAL) && !(Switches & TPCII::SW_DIRECT);
  case TPCII::ST_TNSR_S:
    return Switches & TPCII::SW_HW_REG;
  case TPCII::ST_TNSR_SQZ:
    // The other two forms do not name a tensor.
    return MI.getOpcode() != TPC::ST_TNSR_SQZ_CNT_ONLY &&
           MI.getOpcode() != TPC::ST_TNSR_SQZ_FLUSH;
  default:
    return false;
  }
}

/// A value that identifies the kernel, for the hashedISA field. Like Habana's,
/// it depends on the source text and the file name, so a kernel keeps it across
/// architectures, optimization levels and -S/-c. The values differ from
/// Habana's; the tools run a kernel whatever the value is.
static uint32_t computeKernelHash(const Module &M) {
  // Clang computes it from the main file, see
  // BackendConsumer::HandleTranslationUnit.
  if (const auto *Hash = mdconst::extract_or_null<ConstantInt>(
          M.getModuleFlag(TPCKernelHashFlag)))
    return Hash->getZExtValue();
  // IR from elsewhere: use the source text if an object's .source is there.
  StringRef SourceText;
  if (const GlobalVariable *Source = M.getNamedGlobal("source"))
    if (Source->getSection() == ".source" && Source->hasInitializer())
      if (const auto *Text =
              dyn_cast<ConstantDataSequential>(Source->getInitializer()))
        SourceText = Text->getRawDataValues();
  return computeTPCKernelHash(M.getSourceFileName(), SourceText);
}

char TPCElfSpecSet::ID = 0;

INITIALIZE_PASS_BEGIN(TPCElfSpecSet, PassName, PassDescription, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTreeWrapperPass)
INITIALIZE_PASS_END(TPCElfSpecSet, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCElfSpecSet() { return new TPCElfSpecSet(); }

int TPCElfSpecSet::getLockUnlockIndex(int address) {
  int offset = address & 0xFFF;                    // Register offset
  int TPCBASE = (0x00030000 & address) >> 16;      // TPC ID 0-3
  int DCORE = ((0x00200000 & address) >> 20) == 2; // which DCORE
  int TPCID = TPCBASE + DCORE * 4;                 // Creates unique ID for TPC.
  int addVal = 0;
  switch (offset) {
  case 0xBC4:
    addVal = 0;
    break;
  case 0xBC8:
    addVal = 1;
    break;
  case 0xBCA:
    addVal = 2;
    break;
  case 0xBD0:
    addVal = 3;
    break;
  default:
    return numberOfLockRegister;
  }
  // Each TPCID contains 4 special lock registers.
  // TPCID --> 0,4,8,12,16...
  // addVal connects between the offset to the correct 1D array.
  // Example TPCID 1 and 0xBC8 [1*4+1] --> [5]
  return TPCID * 4 + addVal;
}

void TPCElfSpecSet::setLockUnlockMember(t_LockUnlock SW, int Index) {
  assert((Index <= numberOfLockRegister) &&
         "Trying to access to an array out of bounds");
  SW == LU_LOCK ? lockUnlock[Index].first = 1 : lockUnlock[Index].second = 1;
}

void TPCElfSpecSet::mapInputOutputTensor(const MachineInstr &BMI) {
  bool isImm = false;
  int tensorId = 0;
  const auto &STI = BMI.getParent()->getParent()->getSubtarget<TPCSubtarget>();
  if (TPCII::isLdTnsr(BMI.getDesc(),STI.hasGen3Plus())) {
    if(!BMI.getOperand(2).isImm())
      return;
    tensorId = BMI.getOperand(2).getImm();
    TensorMap[tensorId] = TT_INPUT;
  }
  if (TPCII::isStTnsr(BMI.getDesc())) {
    isImm = BMI.getOperand(1).isImm();
    if (isImm) {
      tensorId = BMI.getOperand(1).getImm();
      TensorMap[tensorId] = TT_OUTPUT;
    }
    if (getSwitches(BMI) & TPCII::SW_RMW_SEL) {
      if (isImm)
        HasRMWStore[tensorId] = true;
      else
        AllTrueRMW = true;
    }
    if (isPartialStore(BMI))
      setTensorBits(TPCMetadata.partialStore, BMI.getOperand(1));
  }
}

void TPCElfSpecSet::detectMMIO(const MachineInstr &BMI) {
  if (TPCInstrInfo::isMMIOAccess(BMI))
    TPCMetadata.mmioUsed = true;
}

void TPCElfSpecSet::searchInstruction(MachineFunction &Func) {
  const auto &STI = Func.getSubtarget<TPCSubtarget>();
  const auto TII = STI.getInstrInfo();
  const auto &MRI = Func.getRegInfo();
  const auto &MDT = getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();

  bool isGoya2 = STI.getFeatureBits()[TPC::FeatureGreco];
  for (auto &MBB : Func) {
    std::vector<const MachineInstr *> DefInstrs;
    MachineBasicBlock::const_instr_iterator MII = MBB.instr_begin();
    for (++MII; MII != MBB.instr_end(); ++MII) {
      const MachineInstr &BMI = *MII;
      mapInputOutputTensor(BMI);
      detectMMIO(BMI);
      if (TPCII::isLookup(BMI.getDesc()) || TPCII::isLookupC(BMI.getDesc()))
        TPCMetadata.specialFunctionUsed = true;

      if (TII->isGenAddr(BMI)) {
        const auto AddrOperand = BMI.getOperand(0);

        const auto AddrReg = AddrOperand.getReg();
        const MachineOperand &Tensor = BMI.getOperand(1);
        // TODO: Ideally we should register data flow analysis, but,
        // apparently ADRF reg is killed with LD use. So this along with
        // dominator check should work.
        for (const auto &MI : MRI.use_instructions(AddrReg)) {
          if (!MDT.dominates(BMI.getParent(), MI.getParent()))
            continue;
          const bool IsLoad = TPCII::is_ld_g(MI.getDesc());
          const bool IsStore = TPCII::is_st_g(MI.getDesc());
          if (IsStore)
            setTensorBits(TPCMetadata.scalarSt, Tensor);
          if (Tensor.isImm() && (IsLoad || IsStore)) {
            TPCMetadata.genAddrTensorMask[Tensor.getImm()] = true;
            if (TII->isGlobalScalarLoad(MI))
              TPCMetadata.scalarLd[Tensor.getImm()] = true;
          }
        }
      }

      // The unlock lock feature is only relevant for goya2.
      if (isGoya2) {
        if (TPCII::is_ld_l(BMI.getDesc()) || TPCII::is_st_l(BMI.getDesc())) {
          // The both switches (LOCK and MMIO) are needed to be set for this
          // feature to take action.
          if ((MII->getOperand(2).getImm() & TPCII::SW_LOCK) &&
              (MII->getOperand(2).getImm() & TPCII::SW_MMIO)) {
            t_LockUnlock operation = TPCII::is_ld_l(BMI.getDesc())
                                         ? LU_LOCK
                                         : LU_UNLOCK;
            // ld_l memory operand 1
            // St_l memory operand 0
            const MachineOperand &Addr = MII->getOperand((operation + 1) % 2);
            int MemberIdx = Addr.isImm() ? getLockUnlockIndex(Addr.getImm()) : numberOfLockRegister;
            setLockUnlockMember(operation, MemberIdx);
          }
        }
      }
    }
  }

}

void TPCElfSpecSet::searchPrintF(MachineFunction &Func) {
  const Module *MM = Func.getFunction().getParent();
  Metadata *PrintfModule = MM->getModuleFlag("tpc-printf");
  if (!PrintfModule)
    return;
  else {
    TPCMetadata.printfUsed = true;
    ConstantAsMetadata *ConstPrintfModule = dyn_cast<ConstantAsMetadata>(
          PrintfModule);
    assert(ConstPrintfModule && "Expect a ConstantAsMetadata");
    ConstantInt *ConstantValue = dyn_cast<ConstantInt>(
          ConstPrintfModule->getValue());
    assert(ConstantValue && "Expect a ConstantInt");
    TPCMetadata.printfTensorID = ConstantValue->getValue()
        .getLimitedValue(std::numeric_limits<uint8_t>::max());
  }
}

void TPCElfSpecSet::CalculateTPCMetadata(const MachineFunction &Func) {
  // Calculate TPCMetadata.paramsNum
 TPCMetadata.paramsNum = Func.getFunction().arg_size();

  // Calculate TPCMetadata.lockUnLock
  for (int i = 0; i < numberOfLockRegister+1; i++) {
    if (LockUnlockMismatchError && (lockUnlock[i].first ^ lockUnlock[i].second)) {
      report_fatal_error("LOCK/UNLOCK doesn't have match UNLOCK/LOCK");
    }
    if (lockUnlock[i].first && lockUnlock[i].second) {
      TPCMetadata.lockUnLock |= true;
    }
  }

  // Calculate TPCMetadata.rmwStore
  for (unsigned j=0; j < TPCNumTensors; j++){
    TPCMetadata.rmwStore[j]= AllTrueRMW || HasRMWStore[j];
  }

  // Calculate TPCMetadata.march
  const TPCSubtarget &SubTarget = Func.getSubtarget<TPCSubtarget>();
  if (SubTarget.hasGoyaISA())
    TPCMetadata.march = 1;
  else if (SubTarget.hasGaudiISA() && !SubTarget.hasGaudiBISA())
    TPCMetadata.march = 2;
  else if (SubTarget.hasGaudiBISA())
    TPCMetadata.march = 5;
  else if (SubTarget.hasGrecoISA())
    TPCMetadata.march = 3;
  else if (SubTarget.hasGaudi2ISA())
    TPCMetadata.march = 4;
  else if (SubTarget.hasDoron1ISA())
    TPCMetadata.march = 6;
  else
    llvm_unreachable("A unhandled march case");

  const TargetOptions &Options =
      SubTarget.getTargetLowering()->getTargetMachine().Options;
  if (Options.TpcDnorm)
    TPCMetadata.dnorm = 1;
  TPCMetadata.irf32Mode = !Options.LongIRF;

  TPCMetadata.hashedISA = computeKernelHash(*Func.getFunction().getParent());
}

/*
static std::vector<Constant *> createAString(std::string input, Type *&Int8Ty) {
  std::vector<Constant *> Init;
  for (unsigned i = 0; i < input.size(); i++) {
    Init.push_back(ConstantInt::get(Int8Ty, input[i]));
  }
  return Init;
}
*/

void TPCElfSpecSet::createElfSection(MachineFunction &Func) {
  llvm::Module *M = Func.getFunction().getParent();
  Type *Int8Ty = llvm::Type::getInt8Ty(Func.getFunction().getContext());

  std::vector<Constant *> TPCMetadataData;
  CalculateTPCMetadata(Func);
  std::vector<uint8_t> BinaryData =
      binarySerializeTPCProgramHeader(TPCMetadata);
  for (const uint8_t &El : BinaryData) {
    TPCMetadataData.push_back(ConstantInt::get(Int8Ty, El));
  }
  ArrayType *ATMetadata = ArrayType::get(Int8Ty, TPCMetadataData.size());
  llvm::GlobalVariable *GV0 =
      new llvm::GlobalVariable(*M, ATMetadata, false,
                               GlobalValue::ExternalLinkage,
                               ConstantArray::get(ATMetadata,
                                                  TPCMetadataData),
                               "tpc_metadata");
  GV0->setSection(BinaryTPCMetadataSectionName);
}

bool TPCElfSpecSet::runOnMachineFunction(MachineFunction &Func) {
  if (!EnableElfSpec)
    return false;
  TPCMetadata.directMMIOAccess =
      Func.getFunction().hasMetadata("tpc.direct.mmio.access");
  searchInstruction(Func);
  searchPrintF(Func);
  createElfSection(Func);
  return false;
}

//===- TPCELFSet.cpp--------Set ELF's TPC Program Header------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// author: Michael Zuckerman
//         mzuckerman@habana.ai
//===----------------------------------------------------------------------===//
// 1) This pass adds metadata to the ELF under the name of TPC_METADATA.
// 2) This header is build according to the protocol of ELFSpec0.1.
// 3) struct {
//      uint32_t version; //spec version
//      uint8_t  specialFunction;
//      uint8_t  printf;
//      uint8_t  lock;
//      bool HasScalarMemOperation[16];
//      int16_t HasRMW; // This indicate what ever there is ST_TNSR_RMW or not
//      uint32_t reserved[256];
//    }
//  4) This pass searches for lookup instruction if exist set
//     report.specialFunction to true.
//  5) This pass searches for printf metadata if exist set printf to true.
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/InitializePasses.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TPCMetadataSection.h"
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
    AU.addRequired<MachineDominatorTree>();
    AU.addPreserved<MachineDominatorTree>();
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

char TPCElfSpecSet::ID = 0;

INITIALIZE_PASS_BEGIN(TPCElfSpecSet, PassName, PassDescription, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
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
  int switchVal = 0;
  const auto &STI = BMI.getParent()->getParent()->getSubtarget<TPCSubtarget>();
  if (TPCII::is_ld_tnsr(BMI.getDesc())) {
    if(!BMI.getOperand(2).isImm())
      return;
    tensorId = BMI.getOperand(2).getImm();
    TensorMap[tensorId] = TT_INPUT;
  }
  if (TPCII::is_st_tnsr(BMI.getDesc())) {
    isImm = BMI.getOperand(1).isImm();
    if (isImm) {
      tensorId = BMI.getOperand(1).getImm();
      TensorMap[tensorId] = TT_OUTPUT;
    }
    if (BMI.getOperand(BMI.getNumOperands() - 2 - 1).isImm())
      switchVal = BMI.getOperand(BMI.getNumOperands() - 2 - 1).getImm();
    else
      switchVal = BMI.getOperand(BMI.getNumOperands() - 2).getImm();
    if (switchVal & TPCII::SW_RMW_SEL) {
      if (isImm)
        HasRMWStore[tensorId] = true;
      else
        AllTrueRMW = true;
    }
  }
}

void TPCElfSpecSet::detectMMIO(const MachineInstr &BMI) {
  if (TPCMetadata.mmioUsed)
    return;

  auto HasMMIOSuffix = [&BMI](unsigned SuffixOpNum) -> bool {
    unsigned SuffixValue = BMI.getOperand(SuffixOpNum).getImm();
    return SuffixValue & TPCII::SW_MMIO;
  };

  if (TPCII::is_ld_l(BMI.getDesc()) && HasMMIOSuffix(2))
    TPCMetadata.mmioUsed = true;
  else if (TPCII::is_st_l(BMI.getDesc())&& HasMMIOSuffix(2))
    TPCMetadata.mmioUsed = true;

}

void TPCElfSpecSet::searchInstruction(MachineFunction &Func) {
  const auto &STI = Func.getSubtarget<TPCSubtarget>();
  const auto TII = STI.getInstrInfo();
  const auto &MRI = Func.getRegInfo();
  const auto &MDT = getAnalysis<MachineDominatorTree>();

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
        if (BMI.getOperand(1).isImm()) {
          const auto TensorID = BMI.getOperand(1).getImm();
          // TODO: Ideally we should register data flow analysis, but,
          // apparently ADRF reg is killed with LD use. So this along with
          // dominator check should work.
          for (const auto &MI : MRI.use_instructions(AddrReg)) {
            if (MDT.dominates(BMI.getParent(), MI.getParent()) &&
                TII->isGlobalScalarLoad(MI)) {
              TPCMetadata.scalarLd[TensorID] = true;
              // Break if ADRF operand is dead or killed.
              if (MI.getOperand(1).isDead() || MI.getOperand(1).isKill())
                break;
            }
          }
        }
      }
    }
  }

}

void TPCElfSpecSet::searchPrintF(MachineFunction &Func) {
  const Module *MM = Func.getMMI().getModule();
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
  for (int j=0; j < TPCNumTensors; j++){
    TPCMetadata.rmwStore[j]= AllTrueRMW || HasRMWStore[j];
  }

  // Calculate TPCMetadata.march
  const TPCSubtarget &SubTarget = Func.getSubtarget<TPCSubtarget>();
  if (SubTarget.hasGoyaISA())
    TPCMetadata.march = 1;
  else if (SubTarget.hasGaudiISA())
    TPCMetadata.march = 2;
  else
    llvm_unreachable("A unhandled march case");
}

static std::vector<Constant *> createAString(std::string input, Type *&Int8Ty) {
  std::vector<Constant *> Init;
  for (unsigned i = 0; i < input.size(); i++) {
    Init.push_back(ConstantInt::get(Int8Ty, input[i]));
  }
  return Init;
}

void TPCElfSpecSet::createElfSection(MachineFunction &Func) {
  llvm::Module *M = const_cast<llvm::Module*>(Func.getMMI().getModule());
  Type *Int8Ty = llvm::Type::getInt8Ty(Func.getFunction().getContext());

  std::vector<Constant *> TPCMetadataData;
  CalculateTPCMetadata(Func);
  std::vector<uint8_t> BinaryData =
      bianrySerializeTPCProgramHeader(TPCMetadata);
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
  searchInstruction(Func);
  searchPrintF(Func);
  createElfSection(Func);
  return false;
}

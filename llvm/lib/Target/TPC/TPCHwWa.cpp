//===---- TPCHwWa.cpp - TPC pre-RA Hardware Workarounds -===//
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"

#include <vector>

#define DEBUG_TYPE "hwwa"

using namespace llvm;
using namespace std;

namespace llvm {
FunctionPass *createTPCPreRAHwWorkarounds();
void initializeTPCPreRAHwWorkaroundsPass(PassRegistry&);
}

static const char PassDescription[] = "TPC pre-RA Hardware Workarounds";
static const char PassName[] = "tpc-pre-ra-hwwa";

static cl::opt<bool>
EnableTPCPreRAHwWorkarounds("tpc-enable-aso-workarounds",
              cl::desc(PassDescription),
              cl::init(true), cl::Hidden);

//
// 0 - Use default (enable both WA for Gaudi, and one WA for Goya2)
// 1 - Disable/Enable SB$ after ASO instruction
// 2 - Issue a CACHE_FLUSH instruction prior to ASO
// 3 - both, 1 and 2
// 4 - Turn off the workarounds
//
static cl::opt<unsigned>
TPC_ASO_WAOpt("tpc-aso-workarounds",
              cl::desc("Disable/Enable ASO workarounds: "
	               "0 - default, 1 - Disable/Enable SB$ after ASO instruction; "
		       "2 - Issue a CACHE_FLUSH instruction prior to ASO; "
		       "3 - both, 1 and 2; 4 - Turn off"),
              cl::init(0), cl::Hidden);

static cl::opt<bool>
TPCCFlushAfterCInvalidateWA("tpc-cache-flush-after-invalidate",
              cl::desc("Insert CACHE_FLUSH after CACHE_INVALIDATE"),
              cl::init(false), cl::Hidden);

static cl::opt<bool>
TPCDisableCFlushAfterCInvalidateWA("tpc-disable-cache-flush-after-invalidate",
              cl::desc("Do not insert CACHE_FLUSH after CACHE_INVALIDATE"),
              cl::init(false), cl::Hidden);

static cl::opt<bool>
InjectMMIOFenceGen3("tpc-mmio-fence-wa",
  cl::desc("Inject MMIO-finished fence workaround for Goya2+"),
  cl::init(true), cl::Hidden);

static cl::opt<bool>
    GCValidation("tpc-mmio-validation", cl::desc("Inject mmio WA validation point"), cl::init(false),cl::Hidden);

namespace {
class TPCPreRAHwWorkarounds : public MachineFunctionPass {
  MachineFunction *MF;
  MachineRegisterInfo *MRI;
  const TargetRegisterInfo *TRI;
  const TargetInstrInfo *TII;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCPreRAHwWorkarounds() : MachineFunctionPass(ID) {
	  initializeTPCPreRAHwWorkaroundsPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool handleMMIOFence(MachineFunction &MF, vector<MachineInstr*> MMIOSeen, MachineInstr *firstMI, MachineInstr *haltMI);
};
}

char TPCPreRAHwWorkarounds::ID = 0;

INITIALIZE_PASS(TPCPreRAHwWorkarounds, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCPreRAHwWorkarounds() {
  return new TPCPreRAHwWorkarounds();
}


bool TPCPreRAHwWorkarounds::runOnMachineFunction(MachineFunction &Func) {
  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnableTPCPreRAHwWorkarounds)
    return false;

  bool needASO_WA1 = false;
  bool needASO_WA2 = false;
  bool needCacheFlAfterCacheInv_WA = false;
  bool needMMIOFence = InjectMMIOFenceGen3 && Func.getSubtarget<TPCSubtarget>().hasGrecoISA();
  
  if (TPC_ASO_WAOpt == 0) { // defaults
    if (Func.getSubtarget<TPCSubtarget>().hasGaudiISA() ||
        Func.getSubtarget<TPCSubtarget>().hasGaudiBISA()) {
      needASO_WA1 = true;
    } else if (Func.getSubtarget<TPCSubtarget>().hasGoyaISA()) {
      needASO_WA2 = true;
    }
  }
  else {
    needASO_WA1 = (TPC_ASO_WAOpt & 1) != 0;
    needASO_WA2 = (TPC_ASO_WAOpt & 2) != 0;
  }

  if (Func.getSubtarget<TPCSubtarget>().hasGaudiISA() ||
      Func.getSubtarget<TPCSubtarget>().hasGaudiBISA()) {
    needCacheFlAfterCacheInv_WA = !TPCDisableCFlushAfterCInvalidateWA;
  } else {
    needCacheFlAfterCacheInv_WA = TPCCFlushAfterCInvalidateWA;
  }

  if (!needASO_WA1 && !needASO_WA2 && !needCacheFlAfterCacheInv_WA && !needMMIOFence)
    return false;

  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  TRI = MF->getSubtarget().getRegisterInfo();

  // Bug LLVM-198:
  // ------------
  // 1) There is a new SB$ contains 4 elements on Gadui.
  //
  // 2) The bug: $ invalid was no issued after vector store evicted
  //    and ASO instruction was shown. 
  //
  // 3) There is no $ invalid instruction for the SB.  
  //
  // 4) Workaround:  After the compiler sees ASO instruction.
  //    The compiler injects "ST_L MMIO 0X99c, 1" to cause SB$
  //    to disable himself and unset ("ST_L MMIO 0X99c, 0")
  //    before the HALT( end of the program)
  //
  // The compiler will insert the following code at the beginning
  // of 'main' function^
  //    int val = s_i32_ld_l_s(0x99C, e_access_mmio);
  //    i32_st_l_s_s(0x99C, val | 0b1,  e_access_mmio);
  //
  // and at the end, before HALT instruction:
  //    i32_st_l_s_s(0x99C, val,  e_access_mmio);
  //
  // Bug LLVM-156:
  // ------------
  // Problem: ASO doesn't flushD$ in case aso_op equals to VPU
  // Implication: Scalars produced by the core won't be globally
  //              observed after ASO
  // Workaround: Issue a CACHE_FLUSH instruction prior to ASO
  //
  bool Changed = false;
  bool ASOSeen = false;
  vector<MachineInstr*> MMIOSeen;
  MachineInstr *firstMI = nullptr;
  MachineInstr *haltMI = nullptr;
  for (MachineFunction::iterator I = MF->begin(), E = MF->end(); I != E; ++I) {
    MachineBasicBlock * MBB = &*I;
    for (MachineBasicBlock::iterator mi = MBB->begin(), me = MBB->end(); mi != me; ) {
      MachineBasicBlock::iterator next_mi = std::next(mi);
      MachineInstr *MI = &*mi;
      unsigned opc = MI->getOpcode();
      if (!firstMI) {
        firstMI = MI;
      }
      if (needMMIOFence && TPCInstrInfo::isMMIOAccess(*MI)) {
        LLVM_DEBUG(dbgs() << " mmio seen: " << *MI);
        MMIOSeen.push_back(MI);
      }
      if (opc == TPC::ASO) {
        ASOSeen = true;
        if (needASO_WA2) {
          BuildMI(*(MI->getParent()), mi, DebugLoc(), TII->get(TPC::CACHE_FLUSH))
            .addImm(0)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
          mi = next_mi;
          Changed = true;
          continue;
        }
      }
      if (opc == TPC::CACHE_INVALIDATE) {
        if (needCacheFlAfterCacheInv_WA) {
          BuildMI(*(MI->getParent()), next_mi, DebugLoc(), TII->get(TPC::CACHE_FLUSH))
            .addImm(0)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
          mi = next_mi;
          Changed = true;
          continue;
        }
      }
      if (opc == TPC::HALTs || opc == TPC::HALTv) {
        haltMI = MI;
      }
      mi++;
    }
  }

  if (ASOSeen && needASO_WA1) {
    assert(firstMI);
    assert(haltMI);
    MachineInstrBuilder MIB;
    MachineBasicBlock::iterator It = firstMI;
    MachineBasicBlock::iterator ItLast = haltMI;

    // ld_l mmio %S0, 0x99c, %SP0
    unsigned s_reg = MRI->createVirtualRegister(&TPC::SRFRegClass);
    MIB = BuildMI(*(firstMI->getParent()), It, DebugLoc(), TII->get(TPC::LD_Lsip), s_reg);
    MIB.addImm(0x99C);                  // MMIO address
    MIB.addImm(TPCII::SW_MMIO);         // MMIO switch
    MIB.addReg(s_reg, RegState::Undef); // income
    MIB.addReg(TPC::SPRF_TRUE);               // Pred
    MIB.addImm(0);                      // Polarity
    if (needMMIOFence)
      MMIOSeen.push_back(MIB.getInstr());

    // or.i32  %S2, %S0, 0x1, %SP0
    unsigned s_reg1 = MRI->createVirtualRegister(&TPC::SRFRegClass);
    MIB = BuildMI(*(firstMI->getParent()), It, DebugLoc(), TII->get(TPC::ORsip), s_reg1);
    MIB.addReg(s_reg);                   // Value
    MIB.addImm(1);                       // Value2
    MIB.addImm(TPCII::OpType::INT32);    // Data type
    MIB.addImm(0);                       // Switch
    MIB.addReg(s_reg1, RegState::Undef); // income
    MIB.addReg(TPC::SPRF_TRUE);                // Pred
    MIB.addImm(0);                       // Polarity

    // st_l mmio 0x99c, %S2, %SP0
    MIB = BuildMI(*(firstMI->getParent()), It, DebugLoc(), TII->get(TPC::ST_Lisp));
    MIB.addImm(0x99C);          // MMIO address
    MIB.addReg(s_reg1);         // Value
    MIB.addImm(TPCII::SW_MMIO); // MMIO switch
    MIB.addReg(TPC::SPRF_TRUE);       // Pred
    MIB.addImm(0);              // Polarity
    if (needMMIOFence)
      MMIOSeen.push_back(MIB.getInstr());

    // Before HALT instruction:
    //    st_l mmio 0x99c, %S0, %SP0
    MIB = BuildMI(*(haltMI->getParent()), ItLast, DebugLoc(), TII->get(TPC::ST_Lisp));
    MIB.addImm(0x99C);          // MMIO address
    MIB.addReg(s_reg);          // Value
    MIB.addImm(TPCII::SW_MMIO); // MMIO switch
    MIB.addReg(TPC::SPRF_TRUE);       // Pred
    MIB.addImm(0);              // Polarity
    if (needMMIOFence)
      MMIOSeen.push_back(MIB.getInstr());

    Changed = true;
  }

  Changed |= handleMMIOFence(Func, MMIOSeen, firstMI, haltMI);

  return Changed;
}

/*
There is a new hardware bug when working with two instants of kernels one by the other.
The bug happens when both QMAN and kernel try to write/read to/from configuration space.
When this happens, the kernel writing is overriding the QMAN writing.

To overcome the above is expected from the compiler to do as follow:
If exist MMIO access, the compiler shall inject an “st_l.mmio %S31,$SXX” to the last execute MMIO instruction (ld_l or st_l).
The $SXX value shall be set to one while Synapse will be responsible for setting $S31 correctly.

Be aware of the problem with loops. MMIO instruction can be executed more than once,
and so the location of the st_l shall be in the exit of all loops.
*/
bool TPCPreRAHwWorkarounds::handleMMIOFence(MachineFunction &MF, vector<MachineInstr*> MMIOSeen, MachineInstr *firstMI, MachineInstr *haltMI) {
  if (MMIOSeen.size() == 0)
    return false;

  //unsigned FenceAddr = MF.addLiveIn(TPC::S31, &TPC::SRFRegClass);
  //firstMI->getParent()->addLiveIn(TPC::S31);
  //BuildMI(*(firstMI->getParent()), firstMI, DebugLoc(), TII->get(TargetOpcode::COPY), FenceAddr).addReg(TPC::S31);
  unsigned FenceAddr = MRI->createVirtualRegister(&TPC::SRFRegClass);
  // unsigned s_reg = MRI->createVirtualRegister(&TPC::SRFRegClass);
  MachineInstrBuilder MIB =
      BuildMI(*(firstMI->getParent()), firstMI, DebugLoc(),
              TII->get(TPC::LD_Lsip), FenceAddr);
  MIB.addImm(0x440);                      // MMIO address
  MIB.addImm(TPCII::SW_MMIO);             // MMIO switch
  MIB.addReg(FenceAddr, RegState::Undef); // income
  MIB.addReg(TPC::SPRF_TRUE);                   // Pred
  MIB.addImm(0);                          // Polarity

  // create an interrupt if GC doesn't work well with semaphore
  if(GCValidation) {
    unsigned FenceActualAddr = MRI->createVirtualRegister(&TPC::SRFRegClass);

/*
    MachineInstrBuilder MIBAdd =
        BuildMI(*(firstMI->getParent()), firstMI, DebugLoc(),
                TII->get(TPC::ADDsip), FenceActualAddr)
            .addReg(FenceAddr)                      // src1
            .addImm(0x50)                           // Address
            .addImm(TPCII::OpType::INT32)           // Type
            .addImm(0)                              // switch
            .addReg(FenceAddr, RegState::Undef)     // income
            .addReg(TPC::SPRF_TRUE)                 // predicate
            .addImm(0);                             // Polarity
*/

    unsigned MOVRefernce = MRI->createVirtualRegister(&TPC::SRFRegClass);
    MachineInstrBuilder MIBLoad =
        BuildMI(*(firstMI->getParent()), firstMI, DebugLoc(),
                TII->get(TPC::LD_Lssp), MOVRefernce);
    MIBLoad.addReg(FenceActualAddr);
    MIBLoad.addImm(TPCII::SW_MMIO);                      // MMIO switch
    MIBLoad.addReg(MOVRefernce, RegState::Undef);        // income
    MIBLoad.addReg(TPC::SPRF_TRUE);                            // Pred
    MIBLoad.addImm(0);                                   // Polarity

    unsigned CMP = MRI->createVirtualRegister(&TPC::SPRFRegClass);
    MachineInstrBuilder MIBCmp =
        BuildMI(*(firstMI->getParent()), firstMI, DebugLoc(),
                TII->get(TPC::CMP_NEQsip), CMP);
    MIBCmp.addReg(MOVRefernce);          // Register (reference value)
    MIBCmp.addImm(0);                    // Compare value
    MIBCmp.addImm(TPCII::OpType::INT32); // Type
    MIBCmp.addImm(0);                    // Switch
    MIBCmp.addReg(CMP, RegState::Undef); // income
    MIBCmp.addReg(TPC::SPRF_TRUE);             // Pred
    MIBCmp.addImm(0);                    // Polarity

    unsigned s_reg1 = MRI->createVirtualRegister(&TPC::SRFRegClass);
    BuildMI(*(firstMI->getParent()), firstMI, DebugLoc(), TII->get(TPC::MOVsip),
            s_reg1)
        .addImm(2048)                    // invalidate location
        .addImm(TPCII::OpType::INT32)    // type
        .addImm(0)                       // switch
        .addReg(s_reg1, RegState::Undef) // income
        .addReg(CMP)                     // Pred
        .addImm(0);                      // Polarity

    BuildMI(*(firstMI->getParent()), firstMI, DebugLoc(),
            TII->get(TPC::ST_Lssp))
        .addReg(s_reg1) // MMIO address
        .addReg(s_reg1) // Value
        .addImm(0)      // MMIO switch
        .addReg(CMP)    // Pred
        .addImm(0);     // Polarity
  }

  // TODO find more accurate postdominator over all exit paths
  // Now before HALT instruction:
  MachineBasicBlock::iterator ItLast = haltMI;
  MachineBasicBlock *MBB = haltMI->getParent();
  //    mov.i32 %S0, 1, %SP0
  unsigned s_reg = MRI->createVirtualRegister(&TPC::SRFRegClass);
  BuildMI(*MBB, ItLast, DebugLoc(), TII->get(TPC::MOVsip), s_reg)
  .addImm(1)                  // value
  .addImm(TPCII::OpType::INT32)
  .addImm(0)         // switch
  .addReg(s_reg, RegState::Undef) // income
  .addReg(TPC::SPRF_TRUE)               // Pred
  .addImm(0);                      // Polarity

  //    st_l mmio S31, %S0, %SP0
  BuildMI(*MBB, ItLast, DebugLoc(), TII->get(TPC::ST_Lssp))
  .addReg(FenceAddr)      // MMIO address
  .addReg(s_reg)          // Value
  .addImm(TPCII::SW_MMIO) // MMIO switch
  .addReg(TPC::SPRF_TRUE)       // Pred
  .addImm(0);             // Polarity

  return true;
}

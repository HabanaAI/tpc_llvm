//===- TPCPrologueMMIO.cpp - gaudi2 kernel prologue -----------------------===//
//
// Habana's compiler starts every gaudi2 kernel with
//
//   event            vpu 0xff            ; every -O level
//   cache_invalidate lu                  ; only in kernels without a lookup
//   ld_l             mmio %Sx, 0x538     ; -O1 and above
//   or.i32           %Sx, %Sx, 0x1000000 ; and.i32 %Sx, %Sx, 0xfeffffff
//   st_l             mmio 0x538, %Sx     ;   with -long-irf
//
// each in a bundle of its own, ahead of any kernel code, and this pass does the
// same. %Sx is the lowest scalar register that does not hold a kernel argument.
// The cache_invalidate is the one lookupWorkAround (TPCHwWaGeneral.cpp) puts at
// the start of the kernel; this pass moves it next to the event.
//
// The pass runs after scheduling and before the packetizer, which gives each
// instruction flagged FrameSetup a VLIW of its own. The latency resolver then
// adds the NOPs between them, and keeps the kernel code 5 cycles behind the
// KERNEL_CONFIG write, as Habana's compiler does.
//
// EVENT sends a trace message for the profiler (llvm/docs/TPC/design/
// EventSupport.rst). -tpc-insert-event-instruction=false leaves it out, as it
// does on Habana's compiler. 0x538 is the KERNEL_CONFIG register.
//
// Bit 24 of KERNEL_CONFIG selects 32-bit IRF arithmetic; with it clear, IRF
// adds carry past bit 31 (the field is IRF_32BIT_COMPATIBILTY in the
// habanalabs driver's gaudi3 register masks, bit 25 there). The runtime
// already sets it for normal kernels and clears it for -long-irf ones, so the
// write only makes sure, and a -long-irf kernel must clear it rather than
// set it. Tested on the simulator and on gaudi2 hardware: the new mode applies
// to the very next bundle and is reset at the next kernel launch.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-prologue-mmio"

static cl::opt<bool>
    InsertEvent("tpc-insert-event-instruction", cl::init(true), cl::Hidden,
                cl::desc("Start every gaudi2 kernel with 'event vpu 0xff'"));

static cl::opt<bool> SetKernelConfig(
    "tpc-prologue-kernel-config", cl::init(true), cl::Hidden,
    cl::desc("Set the IRF mode bit (24) of KERNEL_CONFIG at the start of "
             "every gaudi2 kernel at -O1 and above (clear it with -long-irf)"));

namespace llvm {
FunctionPass *createTPCPrologueMMIO();
void initializeTPCPrologueMMIOPass(PassRegistry &);
} // namespace llvm

namespace {

constexpr unsigned EventData = 0xff;
constexpr unsigned EventVPU = 1;
constexpr unsigned KernelConfigAddr = 0x538;
constexpr unsigned KernelConfigIRF32 = 0x1000000;

class TPCPrologueMMIO : public MachineFunctionPass {
public:
  static char ID;

  TPCPrologueMMIO() : MachineFunctionPass(ID) {
    initializeTPCPrologueMMIOPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "TPC Hardware Prologue MMIO Insertion";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // namespace

char TPCPrologueMMIO::ID = 0;

INITIALIZE_PASS(TPCPrologueMMIO, DEBUG_TYPE,
                "TPC Hardware Prologue MMIO Insertion", false, false)

FunctionPass *llvm::createTPCPrologueMMIO() { return new TPCPrologueMMIO(); }

// The cache_invalidate that lookupWorkAround adds when the kernel has no
// lookup. It flags it FrameSetup on gaudi2.
static MachineInstr *findLookupWorkAround(MachineBasicBlock &Entry) {
  for (MachineInstr &MI : Entry)
    if (MI.getOpcode() == TPC::CACHE_INVALIDATE &&
        MI.getFlag(MachineInstr::FrameSetup))
      return &MI;
  return nullptr;
}

// The lowest scalar register that is not reserved and holds no kernel
// argument. Nothing else is live before the first instruction.
static Register findFreeSRF(const MachineFunction &MF) {
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  const MachineBasicBlock &Entry = MF.front();
  for (MCPhysReg Reg : TPC::SRFRegClass) {
    if (MRI.isReserved(Reg))
      continue;
    bool Used = false;
    for (MCRegAliasIterator AI(Reg, TRI, /*IncludeSelf=*/true); AI.isValid();
         ++AI)
      Used |= MRI.isLiveIn(*AI) || Entry.isLiveIn(*AI);
    if (!Used)
      return Reg;
  }
  return Register();
}

bool TPCPrologueMMIO::runOnMachineFunction(MachineFunction &MF) {
  const auto &ST = MF.getSubtarget<TPCSubtarget>();
  if (!ST.hasGaudi2ISA() || !MF.getFunction().hasFnAttribute("tpc-kernel"))
    return false;

  const TargetInstrInfo *TII = ST.getInstrInfo();
  MachineBasicBlock &Entry = MF.front();
  MachineBasicBlock::iterator Pos = Entry.begin();
  DebugLoc DL = Entry.findDebugLoc(Pos);
  bool Changed = false;

  if (InsertEvent) {
    BuildMI(Entry, Pos, DL, TII->get(TPC::EVENT_ld_ip))
        .addImm(EventData)
        .addImm(EventVPU)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0)
        .setMIFlag(MachineInstr::FrameSetup);
    Changed = true;
  }

  if (MachineInstr *CI = findLookupWorkAround(Entry)) {
    if (CI == &*Pos)
      ++Pos;
    else
      Entry.splice(Pos, &Entry, CI->getIterator());
    Changed = true;
  }

  if (SetKernelConfig && MF.getTarget().getOptLevel() != CodeGenOptLevel::None) {
    Register Tmp = findFreeSRF(MF);
    if (!Tmp)
      report_fatal_error("no free scalar register for the gaudi2 prologue");
    BuildMI(Entry, Pos, DL, TII->get(TPC::LD_Lsip), Tmp)
        .addImm(KernelConfigAddr)
        .addImm(TPCII::SW_MMIO)
        .addReg(Tmp, RegState::Undef) // income
        .addReg(TPC::SPRF_TRUE)
        .addImm(0)
        .setMIFlag(MachineInstr::FrameSetup);
    bool LongIRF = MF.getTarget().Options.LongIRF;
    BuildMI(Entry, Pos, DL, TII->get(LongIRF ? TPC::ANDsip : TPC::ORsip), Tmp)
        .addReg(Tmp)
        .addImm(LongIRF ? int32_t(~KernelConfigIRF32) : KernelConfigIRF32)
        .addImm(TPCII::OpType::INT32)
        .addImm(0)                    // switches
        .addReg(Tmp, RegState::Undef) // income
        .addReg(TPC::SPRF_TRUE)
        .addImm(0)
        .setMIFlag(MachineInstr::FrameSetup);
    BuildMI(Entry, Pos, DL, TII->get(TPC::ST_Lisp))
        .addImm(KernelConfigAddr)
        .addReg(Tmp)
        .addImm(TPCII::SW_MMIO)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0)
        .setMIFlag(MachineInstr::FrameSetup);
    Changed = true;
  }

  return Changed;
}

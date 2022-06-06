//===- TPCMachineScheduler.cpp ---- Custom MI Scheduler for TPC -----------===//
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCMachineScheduler.h"
#include "llvm/CodeGen/LiveInterval.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/RegisterClassInfo.h"
#include "llvm/CodeGen/RegisterPressure.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/ScheduleDAGMutation.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

#define DEBUG_TYPE "tpcsched"

static cl::opt<bool> TPCUseIterativeSched("tpc-enable-iterative-scheduler",
    cl::Hidden, cl::ZeroOrMore, cl::init(true));

static cl::opt<int> VlmSpillsAllowed("tpc-vlm-spills-allowed", cl::Hidden,
    cl::init(-1), cl::desc("How many vector registers can be spilled (default is 16Kb of VLM)."));

static cl::opt<bool> TPCIgnoreBBRegPressure("tpc-ignore-bb-reg-pressure",
    cl::Hidden, cl::ZeroOrMore, cl::init(false));

static cl::opt<bool> TPCUseFullSched("tpc-use-full-sched",
    cl::Hidden, cl::ZeroOrMore, cl::init(false));

static cl::opt<float> RPThreshold("tpc-reg-pressure", cl::Hidden,
    cl::init(0.65f), cl::desc("High register pressure threhold."));

static cl::opt<int> RPLatencyOverlap("tpc-sched-pending", cl::Hidden,
    cl::init(0), cl::desc("Deprecated. Allow schedule pending instructions N cycles ahead."));

static cl::opt<bool> TPCVectorOpsOnLd("tpc-vector-ops-on-ld",
    cl::Hidden, cl::ZeroOrMore, cl::init(true));

#ifndef NDEBUG
static cl::opt<bool> TPCViewMISchedDAGs("tpc-view-sched-dags", cl::Hidden,
  cl::desc("Pop up a window to show MISched dags after they are processed"));
#else
static bool TPCViewMISchedDAGs = false;
#endif

namespace {

/// TPC scheduler.
//
class TPCMachineScheduler : public ScheduleDAGMILive {
public:

  enum : unsigned {
    Collect,
    InitialSchedule,
    Reschedule,
    LastStage = Reschedule
  };

  // Scheduling stage number.
  unsigned Stage;
  bool ScheduleForMinRegs;

  // Current region index.
  size_t RegionIdx;

  // Vector of regions for later rescheduling
  SmallVector<std::pair<MachineBasicBlock::iterator,
                        MachineBasicBlock::iterator>, 32> Regions;

  // Records if a region is not yet scheduled, or schedule has been reverted,
  // or we generally desire to reschedule it.
  BitVector RescheduleRegions;

  TPCMachineScheduler(MachineSchedContext *C,
                      std::unique_ptr<MachineSchedStrategy> S)
    : ScheduleDAGMILive(C, std::move(S)) {
    addMutation(std::make_unique<TPCSubtarget::TPCDAGMutation>());
    Stage = Collect;
    ScheduleForMinRegs = false;
  }

  void schedule() override;
  void finalizeSchedule() override;

  RegisterClassInfo *getRegClassInfo() { return RegClassInfo; }
  int getBBSize() { return BB->size(); }
  MachineBasicBlock* getBB() { return BB; }

  void dumpDAG();
  RegisterPressure& getCurrRegPressure();
};


/// Resource Model
//
class TPCResourceModel {
  /// ResourcesModel - Represents VLIW state.
  DFAPacketizer *ResourcesModel;
  const TargetSchedModel *SchedModel;
  std::vector<SUnit*> Packet;
  unsigned TotalPackets;

public:
    std::vector<SUnit*> OldPacket;

public:
  TPCResourceModel(const TargetSubtargetInfo &STI, const TargetSchedModel *SM)
      : SchedModel(SM), TotalPackets(0) {
  ResourcesModel = STI.getInstrInfo()->CreateTargetScheduleState(STI);

    assert(ResourcesModel && "Unimplemented CreateTargetScheduleState.");

    Packet.resize(SchedModel->getIssueWidth());
    Packet.clear();
    OldPacket.resize(SchedModel->getIssueWidth());
    OldPacket.clear();
    ResourcesModel->clearResources();
  }

  ~TPCResourceModel() {
    delete ResourcesModel;
  }

  void resetPacketState() {
    Packet.clear();
  }

  void resetDFA() {
    ResourcesModel->clearResources();
  }

  void reset() {
    Packet.clear();
    ResourcesModel->clearResources();
  }

  bool isResourceAvailable(SUnit *SU);
  bool reserveResources(SUnit *SU);
  void savePacket();
  unsigned getTotalPackets() const { return TotalPackets; }
  unsigned getCurPacketSize() const { return Packet.size(); }

  bool isInPacket(SUnit *SU) const { return is_contained(Packet, SU); }
};

/// TPC scheduling strategy.
///
class TPCSchedStrategy : public MachineSchedStrategy {
  struct SchedCandidate {
    SUnit *SU;
    RegPressureDelta RPDelta;
    int SCost;
    SchedCandidate(): SU(nullptr), SCost(0) {}
  };

  enum CandResult {
    NoCand, NodeOrder, SingleExcess, SingleCritical, SingleMax, MultiPressure, BestCost};

  struct TPCSchedBoundary {
    TPCMachineScheduler *DAG;
    const TargetSchedModel *SchedModel;

    ReadyQueue Available;

    ScheduleHazardRecognizer *HazardRec;
    TPCResourceModel *ResourceModel;

    unsigned CurrCycle;
    unsigned IssueCount;
    unsigned MinReadyCycle;
    unsigned MaxMinLatency;
    unsigned CriticalPathLength = 0;
    int NumADRFregs = 0;

    TPCSchedBoundary(unsigned ID, const Twine &Name):
      DAG(nullptr), SchedModel(nullptr), Available(ID, Name+".A"),
      HazardRec(nullptr), ResourceModel(nullptr),
      CurrCycle(0), IssueCount(0),
      MinReadyCycle(UINT_MAX), MaxMinLatency(0) {}

    ~TPCSchedBoundary() {
      delete ResourceModel;
      delete HazardRec;
    }

    void init(TPCMachineScheduler *dag, const TargetSchedModel *smodel, bool postRA) {
      DAG = dag;
      SchedModel = smodel;
      IssueCount = 0;
      CurrCycle = 0;

      CriticalPathLength = 1;
      unsigned int bbApproxSize = 0;
      for (auto &SU : DAG->SUnits) {
        if (SU.getInstr()->isPseudo() && !SU.getInstr()->isCopy()) {
          continue;
        }
        bbApproxSize++;
      }
      bool schedForMinRegs = false;
      if (!postRA) {
        schedForMinRegs = dag->ScheduleForMinRegs;
      }
      // Initialize the critical path length limit, which used by the scheduling
      // cost model to determine the value for scheduling an instruction. We use
      // a slightly different heuristic for small and large functions. For small
      // functions, it's important to use the height/depth of the instruction.
      // For large functions, prioritizing by height or depth increases spills.
      if (!schedForMinRegs) {
        if (bbApproxSize < 100) {
          CriticalPathLength >>= 1;
          return;
        }

        // For large basic blocks, we prefer a larger critical path length to
        // decrease the priority of using the graph height/depth.
        unsigned MaxPath = 0;
        for (auto &SU : DAG->SUnits)
          MaxPath = std::max(MaxPath, isTop() ? SU.getHeight() : SU.getDepth());
        CriticalPathLength = std::max(CriticalPathLength, MaxPath) + 1;
      }
      else {
        unsigned MaxPath = 0;
        for (auto &SU : DAG->SUnits)
          MaxPath = std::max(MaxPath, isTop() ? SU.getHeight() : SU.getDepth());
        if (bbApproxSize < 100) {
          CriticalPathLength = bbApproxSize >> 1;
          return;
        }
        if (bbApproxSize > MaxPath) {
          CriticalPathLength = bbApproxSize - MaxPath - 1;
        }
        else {
          CriticalPathLength = MaxPath - 1;
        }
      }
    }


    bool isTop() const {
      return Available.getID() == TPCSchedStrategy::TopQID;
    }

    const char * getName() const {
      return (Available.getID() == TPCSchedStrategy::TopQID) ? "Top" : "Bot";
    }

    bool checkHazard(SUnit *SU);
    void releaseNode(SUnit *SU, unsigned ReadyCycle, bool isPostRA);
    void bumpCycle();
    void bumpNode(SUnit *SU, bool isPostRA);
    void removeReady(SUnit *SU);
    SUnit *pickOnlyChoice();

    bool isLatencyBound(SUnit *SU) {
      if (CurrCycle >= CriticalPathLength)
        return true;
      unsigned PathLength = isTop() ? SU->getHeight() : SU->getDepth();
      return CriticalPathLength - CurrCycle <= PathLength;
    }
  };

  TPCMachineScheduler *DAG;
  const TargetSchedModel *SchedModel;
  TPCSchedBoundary Top;
  TPCSchedBoundary Bot;
  SUnit * lastScheduledSU;
  std::vector<bool> HighPressureSets;
  int liveInsADRF;
public:
  bool isPostRA;
  enum {
    TopQID = 1,
    BotQID = 2,
    LogMaxQID = 2
  };
public:
  TPCSchedStrategy()
    : DAG(nullptr), SchedModel(nullptr), Top(TopQID, "TopQ"), Bot(BotQID, "BotQ"), lastScheduledSU(nullptr) {isPostRA = false;}
  ~TPCSchedStrategy() override = default;
  void initialize(ScheduleDAGMI *dag) override;
  SUnit *pickNode(bool &IsTopNode) override;
  void schedNode(SUnit *SU, bool IsTopNode) override;
  void releaseTopNode(SUnit *SU) override;
  void releaseBottomNode(SUnit *SU) override;
protected:
  int SchedulingCost(ReadyQueue &Q,
                     SUnit *SU, SchedCandidate &Candidate,
                     RegPressureDelta &Delta, bool verbose);

  CandResult pickNodeFromQueue(ReadyQueue &Q,
                               const RegPressureTracker &RPTracker,
                               SchedCandidate &Candidate);
  SUnit *pickNodeBidrectional(bool &IsTopNode);

  bool tryChangeSlot(SUnit *SU, bool do_change);
  int pressureChange(const SUnit *SU, bool isBotUp);
  unsigned getRegPressure(SUnit *SU, const TargetRegisterClass *RC);

  bool shouldTrackLaneMasks() const override { return true; }
};

class TPCPostRASchedStrategy : public TPCSchedStrategy {
public:
  TPCPostRASchedStrategy()
    : TPCSchedStrategy() {isPostRA = true;}
};

}

static inline bool isDefADRF(SUnit *SU) {
  unsigned Opcode = SU->getInstr()->getOpcode();
  if (Opcode == TPC::GEN_ADDR_ld || Opcode == TPC::GEN_ADDR_ldT ||
      Opcode == TPC::GEN_ADDR_st || Opcode == TPC::GEN_ADDR_stT) {
    return true;
  }
  return false;
}

#if 0
static bool isHWReg(const MachineInstr &MI, unsigned OpNo) {
  const llvm::MachineFunction *MF = MI.getParent()->getParent();
  Register Reg = MI.getOperand(0).getReg();

  const TargetRegisterClass *TRC;
  if (Reg.isVirtual()) {
    const MachineRegisterInfo &MRI = MF->getRegInfo();
    TRC = MRI.getRegClass(Reg);
  } else {
    const TargetSubtargetInfo &STI = MF->getSubtarget();
    const TPCInstrInfo *TII = static_cast<const TPCInstrInfo *>(STI.getInstrInfo());
    const TPCRegisterInfo &RI = TII->getRegisterInfo();
    const MCInstrDesc &DefMCID = MI.getDesc();
    TRC = TII->getRegClass(DefMCID, 0, &RI, *MF);
  }
  return TRC->hasSuperClassEq(&TPC::HSRFRegClass) ||
         TRC->hasSuperClassEq(&TPC::HVRFRegClass);
}

static bool isHWRegProducer(const MachineInstr &MI) {
  if (MI.getNumDefs() == 1 && MI.getOperand(0).isReg())
    return isHWReg(MI, 0);
  return false;
}
#endif

unsigned TPCSchedStrategy::getRegPressure(SUnit *SU, const TargetRegisterClass *RC) {
  unsigned res = 0;
  const llvm::MachineFunction &MF = *SU->getInstr()->getParent()->getParent();
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  const RegPressureTracker &TopRPTracker = DAG->getTopRPTracker();
  RegPressureTracker &RPTracker = const_cast<RegPressureTracker&>(TopRPTracker);
  const int * rcps = TRI.getRegClassPressureSets(RC);

  std::vector<unsigned> pressure;
  std::vector<unsigned> MaxPressure;
  RPTracker.getDownwardPressure(SU->getInstr(), pressure, MaxPressure);
  int j = 0;
  while (rcps[j] >= 0 ) {
    if (res < pressure[rcps[j]]) res = pressure[rcps[j]];
    j++;
  }
  if (RC == &TPC::ADRFRegClass) {
    res += liveInsADRF;
  }
  return res;
}

//------------------------------------------------------------------------------
// Implementation of TPC Resource Model
//------------------------------------------------------------------------------

void TPCResourceModel::savePacket() {
  OldPacket = Packet;
}

// Keep track of available resources.
bool TPCResourceModel::reserveResources(SUnit *SU) {
  bool startNewCycle = false;

  if (!SU) {
    ResourcesModel->clearResources();
    savePacket();
    Packet.clear();
    TotalPackets++;
    return false;
  }
  // If this SU does not fit in the packet
  // start a new one.
  if (!isResourceAvailable(SU)) {
    ResourcesModel->clearResources();
    savePacket();
    Packet.clear();
    TotalPackets++;
    startNewCycle = true;
  }

  if (!SU->getInstr()->isPseudo()) {
    ResourcesModel->reserveResources(*SU->getInstr());
    Packet.push_back(SU);
  }

#ifndef NDEBUG
  LLVM_DEBUG(dbgs() << "Packet[" << TotalPackets << "]:\n");
  for (unsigned i = 0, e = Packet.size(); i != e; ++i) {
    LLVM_DEBUG(dbgs() << "\t[" << i << "] SU(");
    LLVM_DEBUG(dbgs() << Packet[i]->NodeNum << ")\t");
    LLVM_DEBUG(Packet[i]->getInstr()->dump());
  }
#endif

  // If packet is now full, reset the state so in the next cycle
  // we start fresh.
  if (Packet.size() >= 4 /*SchedModel->getIssueWidth()*/) {
    ResourcesModel->clearResources();
    savePacket();
    Packet.clear();
    TotalPackets++;
    startNewCycle = true;
  }

  return startNewCycle;
}

/// Check if scheduling of this SU is possible in the current packet.
bool TPCResourceModel::isResourceAvailable(SUnit *SU) {
  if (!SU || !SU->getInstr())
    return false;

  // First see if the pipeline could receive this instruction
  // in the current cycle.
  if (!SU->getInstr()->isPseudo()) {
    if (!ResourcesModel->canReserveResources(*SU->getInstr()))
      return false;
  }

  // Now see if there are no other dependencies to instructions already
  // in the packet.
  for (unsigned i = 0, e = Packet.size(); i != e; ++i) {
    if (Packet[i]->Succs.size() == 0)
      continue;

    for (SUnit::const_succ_iterator I = Packet[i]->Succs.begin(),
         E = Packet[i]->Succs.end(); I != E; ++I) {
      if (I->isCtrl())
        continue;

      if (I->getSUnit() == SU)
        return false;
    }
  }
  return true;
}


//------------------------------------------------------------------------------
// Implementation of TPCMachineScheduler
//------------------------------------------------------------------------------
RegisterPressure& TPCMachineScheduler::getCurrRegPressure() {
  const RegPressureTracker &TopRPTracker = getTopRPTracker();
  RegPressureTracker &RPTracker = const_cast<RegPressureTracker&>(TopRPTracker);
  return RPTracker.getPressure();
}

void TPCMachineScheduler::schedule() {
  //
  // At the first pass we only record regions and do not perform actual scheduling.
  // Actual scheduling will start when finalizeSchedule() is called - it will
  // call this function several times, with 'Stage' other than 'Collect'.
  //
  if (TPCUseIterativeSched) {
    if (Stage == Collect) {
      LLVM_DEBUG(dbgs() << "Collecting regions for scheduling BB#"
                        << BB->getNumber() << "\n");
      Regions.push_back(std::make_pair(RegionBegin, RegionEnd));
      return;
    }
  }

  //
  // Record all instructions in their original order so that
  // the order can be restored later for rescheduling.
  //
  std::vector<MachineInstr*> Unsched;
  Unsched.reserve(NumRegionInstrs);
  for (auto &I : *this) {
    Unsched.push_back(&I);
  }

  //
  // Do actual scheduling
  //
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();

  LLVM_DEBUG(dbgs()
        << "********** TPC MI Scheduling BB#" << BB->getNumber()
        << " " << BB->getName()
        << " in_func " << BB->getParent()->getFunction().getName()
        << " at loop depth " << MLI->getLoopDepth(BB)
        << " \n");
  LLVM_DEBUG(SchedImpl->dumpPolicy());

  buildDAGWithRegPressure();

  postprocessDAG();

  SmallVector<SUnit*, 8> TopRoots, BotRoots;
  findRootsAndBiasEdges(TopRoots, BotRoots);

  // Initialize the strategy before modifying the DAG.
  SchedImpl->initialize(this);

  LLVM_DEBUG(unsigned maxH = 0;
        for (unsigned su = 0, e = SUnits.size(); su != e; ++su)
          if (SUnits[su].getHeight() > maxH)
            maxH = SUnits[su].getHeight();
        dbgs() << "Max Height " << maxH << "\n";);
  LLVM_DEBUG(unsigned maxD = 0;
        for (unsigned su = 0, e = SUnits.size(); su != e; ++su)
          if (SUnits[su].getDepth() > maxD)
            maxD = SUnits[su].getDepth();
        dbgs() << "Max Depth " << maxD << "\n";);

  // Dump list of nodes in short format
  LLVM_DEBUG(
    for (unsigned su = 0, e = SUnits.size(); su != e; ++su)
      dumpNode(SUnits[su]);
  );

  // Dump list of nodes in details
  LLVM_DEBUG(
    dbgs() << "--- Scheduled DAG ---\n";
    dump();
    dbgs() << "--- End of scheduled DAG ---\n";
  );

  // Generate graphical view of the DAGs
  if (TPCViewMISchedDAGs) viewGraph();

  initQueues(TopRoots, BotRoots);

  LLVM_DEBUG({
    auto pressureBeforeSched = getCurrRegPressure();
    dbgs() << "=== Register Pressure before scheduling BB#"
           << BB->getNumber() << " " << BB->getName()
           << " =====\n";
    pressureBeforeSched.dump(TRI);
  });

  bool IsTopNode = false;
  while (true) {
    SUnit *SU = SchedImpl->pickNode(IsTopNode);
    if (!SU) break;

    if (!checkSchedLimit())
      break;

    scheduleMI(SU, IsTopNode);

    // Notify the scheduling strategy after updating the DAG.
    SchedImpl->schedNode(SU, IsTopNode);

    updateQueues(SU, IsTopNode);
  }

  LLVM_DEBUG({
    unsigned BBNum = begin()->getParent()->getNumber();
    dbgs() << "*** Final schedule for BB#" << BBNum << " ***\n";
    dumpSchedule();
    dbgs() << '\n'
           << "**********\n";
  });

  if (!TPCUseIterativeSched) {
    placeDebugValues();
    return;
  }

  //
  // --- Initial scheduling done --- //
  //

  Regions[RegionIdx] = std::make_pair(RegionBegin, RegionEnd);
  RescheduleRegions[RegionIdx] = false;

  if (!LIS)
    return;

  auto pressureAfterSched = getCurrRegPressure();
  LLVM_DEBUG({
    dbgs() << "=== Register Pressure after scheduling BB#"
           << BB->getNumber() << " " << BB->getName()
           << " =====\n";
    pressureAfterSched.dump(TRI);
  });

  //
  // Check whether the scheduling done meets regpressure criteria.
  //
  bool tooManyRegs = false;
  RegisterClassInfo * RCI = getRegClassInfo();
  int liveInVRF = 0;
  for (const auto &RegMaskPair : pressureAfterSched.LiveInRegs) {
    Register Reg = RegMaskPair.RegUnit;
    if (Reg.isPhysical())
      continue;
    const TargetRegisterClass* rClass = MF.getRegInfo().getRegClass(Reg);
    if (rClass == &TPC::VRFRegClass) {
      liveInVRF++;
    }
    /*
    else if (rClass == &TPC::DRFRegClass) {
      liveInVRF+=2;
    }
    else if (rClass == &TPC::ARFRegClass) {
      liveInVRF+=4;
    }
    */
  }
  unsigned VlmLimit  = RCI->getRegPressureSetLimit(TPC::RegisterPressureSets::VRF);
  unsigned VlmSpills = VlmSpillsAllowed;
  if (VlmSpillsAllowed < 0) {
    TPCFrameLowering &FL = *const_cast<TPCFrameLowering *>(
      MF.getSubtarget<TPCSubtarget>().getFrameLowering());
    unsigned MaxVlm = FL.getMaxVectorMemory();
    unsigned VlmUsed = FL.getVectorDataSize();
    LLVM_DEBUG(dbgs() << " MaxVlm   =" << MaxVlm << " (" << (MaxVlm / 256) << " VRF)\n");
    LLVM_DEBUG(dbgs() << "VlmUsed   =" << VlmUsed << " (" << (VlmUsed / 256) << " VRF)\n");
    VlmSpills = ((MaxVlm - VlmUsed) / 256) - 1;
  }
  unsigned curVrfPressure = pressureAfterSched.MaxSetPressure[TPC::RegisterPressureSets::VRF];
  unsigned curDrfPressure = pressureAfterSched.MaxSetPressure[TPC::RegisterPressureSets::DRF];
  unsigned VectorPressure = std::max(curVrfPressure, curDrfPressure);
  int VlmOccupancy = liveInVRF + (int)VectorPressure - (int)VlmLimit;
  //int VlmOccupancy = (int)VectorPressure - (int)VlmLimit;
  LLVM_DEBUG(dbgs() << "   Limit       =" << VlmLimit << "\n");
  LLVM_DEBUG(dbgs() << "   Pressure    =" << VectorPressure << "\n");
  LLVM_DEBUG(dbgs() << "   Occupancy   =" << VlmOccupancy << "\n");
  LLVM_DEBUG(dbgs() << "   SpillsAvail =" << VlmSpills << "\n");
  LLVM_DEBUG(dbgs() << "   liveIn      =" << liveInVRF << "\n");
  if (VlmOccupancy > (int)VlmSpills) {
    LLVM_DEBUG(dbgs() << "Pressure exceed limit:\n");
    LLVM_DEBUG(dbgs() << "    " << "VRF"
           << "=" << pressureAfterSched.MaxSetPressure[TPC::RegisterPressureSets::VRF]
           << " (limit=" << VlmLimit << ")\n");
    tooManyRegs = true;
  }

  if (!tooManyRegs) {
    LLVM_DEBUG(dbgs() << "Pressure in desired limits, done BB#" << BB->getNumber() << "\n");
    LLVM_DEBUG(dbgs() << "    ... at stage " << Stage << "\n");
    placeDebugValues();
    return;
  }

  if (Stage == LastStage) {
    // This is the last stage - we will not restore the original order
    // even if the scheduling done does not meet regpressure criteria.
    // If we do not stop here and restore the original order as final order
    // the code can be so bad that the number of VLIW instructions in a loop block
    // will be more than allowed (the number must fit in 16-bit field)
    placeDebugValues();
    return;
  } 

  //
  // The scheduling done does not meet regpressure criteria.
  // Restore the original order and flags.
  //
  LLVM_DEBUG(dbgs() << "Attempting to revert scheduling BB#" << BB->getNumber() << "\n");
  initRegPressure();
  RescheduleRegions[RegionIdx] = true;
  RegionEnd = RegionBegin;
  for (MachineInstr *MI : Unsched) {

    if (MI->getIterator() != RegionEnd) {
      BB->remove(MI);
      BB->insert(RegionEnd, MI);
      if (!MI->isDebugInstr())
        LIS->handleMove(*MI, true);
    }

    // Reset read-undef flags and update them later.
    for (auto &Op : MI->operands()) {
      if (Op.isReg() && Op.isDef()) {
        Op.setIsUndef(false);
      }
    }

    RegisterOperands RegOpers;
    RegOpers.collect(*MI, *TRI, MRI, ShouldTrackLaneMasks, false);
    if (!MI->isDebugInstr()) {
      if (ShouldTrackLaneMasks) {
        // Adjust liveness and add missing dead+read-undef flags.
        SlotIndex SlotIdx = LIS->getInstructionIndex(*MI).getRegSlot();
        RegOpers.adjustLaneLiveness(*LIS, MRI, SlotIdx, MI);
      } else {
        // Adjust for missing dead-def flags.
        RegOpers.detectDeadDefs(*MI, *LIS);
      }
    }
    RegionEnd = MI->getIterator();
    ++RegionEnd;
    LLVM_DEBUG(dbgs() << "Scheduling " << *MI);
  }
  RegionBegin = Unsched.front()->getIterator();
  Regions[RegionIdx] = std::make_pair(RegionBegin, RegionEnd);

  placeDebugValues();
}

void TPCMachineScheduler::finalizeSchedule() {
  if (!TPCUseIterativeSched) {
    return;
  }
  LLVM_DEBUG(dbgs() << "Starting actual scheduling.\n");

  RescheduleRegions.resize(Regions.size());
  RescheduleRegions.set();

  do {
    Stage++;
    RegionIdx = 0;
    MachineBasicBlock *MBB = nullptr;

    if (Stage > InitialSchedule) {
      if (!LIS)
        break;

      // Retry the scheduling.
      if (Stage == Reschedule) {
        ScheduleForMinRegs = true;
        if (RescheduleRegions.none())
          continue;
        LLVM_DEBUG(dbgs() << "Retrying function scheduling.\n");
      }
    }

    // Code below is partially copied from MachineSchedulerBase::scheduleRegions().
    for (auto Region : Regions) {
      if (Stage == Reschedule && !RescheduleRegions[RegionIdx]) {
        ++RegionIdx;
        continue;
      }

      RegionBegin = Region.first;
      RegionEnd = Region.second;

      if (RegionBegin->getParent() != MBB) {
        if (MBB) finishBlock();
        MBB = RegionBegin->getParent();
        startBlock(MBB);
      }

      unsigned NumRegionInstrs = std::distance(begin(), end());
      enterRegion(MBB, begin(), end(), NumRegionInstrs);

      // Skip empty scheduling regions (0 or 1 schedulable instructions).
      if (begin() == end() || begin() == std::prev(end())) {
        exitRegion();
        continue;
      }

      LLVM_DEBUG(dbgs() << "********** TPC MI Scheduling **********\n");
      LLVM_DEBUG(dbgs() << MF.getName() << ":" << printMBBReference(*MBB) << " "
                        << MBB->getName() << "\n  From: " << *begin()
                        << "    To: ";
                 if (RegionEnd != MBB->end()) dbgs() << *RegionEnd;
                 else dbgs() << "End";
                 dbgs() << " RegionInstrs: " << NumRegionInstrs << '\n');

      schedule();

      exitRegion();
      ++RegionIdx;
    }
    finishBlock();
    ScheduleForMinRegs = false;
  } while (Stage != LastStage);
}


void TPCMachineScheduler::dumpDAG() {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  dbgs() << "------ Schedule DAG ------\n";
  for (MachineBasicBlock::iterator MII = RegionEnd, MIE = RegionBegin;
       MII != MIE; --MII) {
    MachineInstr &MI = *std::prev(MII);
    dbgs() << "  Instr: " << MI;
    SUnit *SU = getSUnit(&MI);
    dbgs() << "  Preds:\n";
    for (const SDep &PredD : SU->Preds) {
      dbgs() << "    ";
      PredD.dump(&TRI);
      dbgs() << " ";
      SUnit *PredSU = PredD.getSUnit();
      MachineInstr *PredMI = PredSU->getInstr();
      if (PredMI)
        PredMI->dump();
    }
    dbgs() << "  Succs:\n";
    for (const SDep &SuccD : SU->Succs) {
      dbgs() << "    ";
      SuccD.dump(&TRI);
      dbgs() << " ";
      SUnit *SuccSU = SuccD.getSUnit();
      MachineInstr *SuccMI = SuccSU->getInstr();
      if (SuccMI)
        SuccMI->dump();
    }
    dbgs() << "----\n";
  }
  dbgs() << "------ /Schedule DAG ------\n";
}


//------------------------------------------------------------------------------
// Implementation of TPCSchedStrategy
//------------------------------------------------------------------------------

void TPCSchedStrategy::initialize(ScheduleDAGMI *dag) {
  LLVM_DEBUG(dbgs() << "TPCSchedStrategy :: initialize\n");
  DAG = static_cast<TPCMachineScheduler*>(dag);
  SchedModel = DAG->getSchedModel();

  Top.init(DAG, SchedModel, isPostRA);
  Bot.init(DAG, SchedModel, isPostRA);

  // Initialize the HazardRecognizers. If itineraries don't exist, are empty, or
  // are disabled, then these HazardRecs will be disabled.
  const InstrItineraryData *Itin = DAG->getSchedModel()->getInstrItineraries();
  const TargetSubtargetInfo &STI = DAG->MF.getSubtarget();
  const TargetInstrInfo *TII = STI.getInstrInfo();

  if (!TPCUseFullSched) {
    llvm::ForceTopDown = true;
  }

  delete Top.HazardRec;
  delete Bot.HazardRec;
  Top.HazardRec = TII->CreateTargetMIHazardRecognizer(Itin, DAG);
  Bot.HazardRec = TII->CreateTargetMIHazardRecognizer(Itin, DAG);

  delete Top.ResourceModel;
  delete Bot.ResourceModel;
  Top.ResourceModel = new TPCResourceModel(STI, DAG->getSchedModel());
  Bot.ResourceModel = new TPCResourceModel(STI, DAG->getSchedModel());

  if (!isPostRA) {
    const std::vector<unsigned> &MaxPressure =
      DAG->getRegPressure().MaxSetPressure;
    HighPressureSets.assign(MaxPressure.size(), 0);
    for (unsigned i = 0, e = MaxPressure.size(); i < e; ++i) {
      unsigned Limit = DAG->getRegClassInfo()->getRegPressureSetLimit(i);
      HighPressureSets[i] =
        ((float) MaxPressure[i] > ((float) Limit * RPThreshold));
      //dbgs() << "   MaxPressure[" << i << "] = " << MaxPressure[i] << "\n";
    }

    liveInsADRF = 0;
    const llvm::MachineFunction &MF = DAG->MF;
    // const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
    for (const auto &RegMaskPair : DAG->getTopRPTracker().getPressure().LiveInRegs) {
      Register Reg = RegMaskPair.RegUnit;
      if (Reg.isPhysical())
        continue;
      const TargetRegisterClass* rClass = MF.getRegInfo().getRegClass(Reg);
      if (rClass == &TPC::ADRFRegClass) {
        liveInsADRF++;
      }
    }
    //if(liveInsADRF) dbgs() << "BB#" << DAG->getBB()->getNumber() << " - liveInsADRF = " << liveInsADRF << "\n";
  }
}


/// Pick the best candidate node from either the top or bottom queue.
SUnit *TPCSchedStrategy::pickNodeBidrectional(bool &IsTopNode) {
  // Schedule as far as possible in the direction of no choice. This is most
  // efficient, but also provides the best heuristics for CriticalPSets.
  if (SUnit *SU = Bot.pickOnlyChoice()) {
    LLVM_DEBUG(dbgs() << "Picked only Bottom\n");
    IsTopNode = false;
    return SU;
  }
  if (SUnit *SU = Top.pickOnlyChoice()) {
    LLVM_DEBUG(dbgs() << "Picked only Top\n");
    IsTopNode = true;
    return SU;
  }
  SchedCandidate BotCand;
  // Prefer bottom scheduling when heuristics are silent.
  CandResult BotResult = pickNodeFromQueue(Bot.Available,
                                           DAG->getBotRPTracker(), BotCand);
  assert(BotResult != NoCand && "failed to find the first candidate");

  if (BotResult == SingleExcess || BotResult == SingleCritical) {
    LLVM_DEBUG(dbgs() << "Prefered Bottom Node\n");
    IsTopNode = false;
    return BotCand.SU;
  }
  // Check if the top Q has a better candidate.
  SchedCandidate TopCand;
  CandResult TopResult = pickNodeFromQueue(Top.Available,
                                           DAG->getTopRPTracker(), TopCand);
  assert(TopResult != NoCand && "failed to find the first candidate");

  if (TopResult == SingleExcess || TopResult == SingleCritical) {
    LLVM_DEBUG(dbgs() << "Prefered Top Node\n");
    IsTopNode = true;
    return TopCand.SU;
  }
  // If either Q has a single candidate that minimizes pressure above the
  // original region's pressure pick it.
  if (BotResult == SingleMax) {
    LLVM_DEBUG(dbgs() << "Prefered Bottom Node SingleMax\n");
    IsTopNode = false;
    return BotCand.SU;
  }
  if (TopResult == SingleMax) {
    LLVM_DEBUG(dbgs() << "Prefered Top Node SingleMax\n");
    IsTopNode = true;
    return TopCand.SU;
  }
  if (TopCand.SCost > BotCand.SCost || BotCand.SU->getInstr()->isPseudo()) {
    LLVM_DEBUG(dbgs() << "Prefered Top Node Cost\n");
    IsTopNode = true;
    return TopCand.SU;
  }
  // Otherwise prefer the bottom candidate in node order.
  LLVM_DEBUG(dbgs() << "Prefered Bottom in Node order\n");
  IsTopNode = false;
  return BotCand.SU;
}

SUnit *TPCSchedStrategy::pickNode(bool &IsTopNode) {
  LLVM_DEBUG(dbgs() << "\n*** pickNode\n");
  SUnit *SU = nullptr;

  if (DAG->top() == DAG->bottom()) {
    assert(Top.Available.empty() && "ReadyQ garbage");
    return nullptr;
  }

  if (llvm::ForceTopDown) {
    SU = Top.pickOnlyChoice();
    if (!SU) {
      SchedCandidate TopCand;
      CandResult TopResult =
        pickNodeFromQueue(Top.Available, DAG->getTopRPTracker(), TopCand);
      assert(TopResult != NoCand && "failed to find the first candidate");
      (void)TopResult;
      SU = TopCand.SU;
    }
    IsTopNode = true;
  }
  else if (llvm::ForceBottomUp) {
    SU = Bot.pickOnlyChoice();
    if (!SU) {
      SchedCandidate BotCand;
      CandResult BotResult =
        pickNodeFromQueue(Bot.Available, DAG->getBotRPTracker(), BotCand);
      assert(BotResult != NoCand && "failed to find the first candidate");
      (void)BotResult;
      SU = BotCand.SU;
    }
    IsTopNode = false;
  } else {
    SU = pickNodeBidrectional(IsTopNode);
  }


  if (SU->isTopReady())
    Top.removeReady(SU);
  if (SU->isBottomReady())
    Bot.removeReady(SU);

  LLVM_DEBUG(
    if (SU) {
      dbgs() << "*** Pick node "
             << (IsTopNode ? "Top" : "Bottom")
             << " at cycle " << (IsTopNode ? Top.CurrCycle : Bot.CurrCycle)
             << '\n';
      dbgs() << "   ";
      DAG->dumpNode(*SU);
    } else {
      dbgs() << "  ** NO NODE \n";
      for (unsigned i = 0; i < DAG->SUnits.size(); i++) {
        const SUnit &S = DAG->SUnits[i];
        if (!S.isScheduled)
          DAG->dumpNode(S);
      }
    }
  );

  if (!isPostRA && IsTopNode) {
    Top.NumADRFregs = 1 + getRegPressure(SU, &TPC::ADRFRegClass);
  }

  return SU;
}

void TPCSchedStrategy::schedNode(SUnit *SU, bool IsTopNode) {
  MachineInstr *MI = SU->getInstr();
  (void)MI;
  lastScheduledSU = SU;
  if (IsTopNode) {
    Top.bumpNode(SU, isPostRA);
    SU->TopReadyCycle = Top.CurrCycle;
    LLVM_DEBUG(dbgs() << "   ** TopReadyCycle = " << SU->TopReadyCycle << "\n");
  } else {
    Bot.bumpNode(SU, isPostRA);
    SU->BotReadyCycle = Bot.CurrCycle;
    LLVM_DEBUG(dbgs() << "   ** BotReadyCycle = " << SU->BotReadyCycle << "\n");
  }
}

void TPCSchedStrategy::releaseTopNode(SUnit *SU) {
  MachineInstr *MI = SU->getInstr();
  (void)MI;
  if (SU->isScheduled) {
    LLVM_DEBUG(dbgs() << "*** release Top - already scheduled: " << *MI);
    return;
  }

  SU->TopReadyCycle = 0;
  for (const SDep &PI : SU->Preds) {
    unsigned PredReadyCycle = PI.getSUnit()->TopReadyCycle;
    unsigned MinLatency = PI.getLatency();
#ifndef NDEBUG
    Top.MaxMinLatency = std::max(MinLatency, Top.MaxMinLatency);
#endif
    if (SU->TopReadyCycle < PredReadyCycle + MinLatency)
      SU->TopReadyCycle = PredReadyCycle + MinLatency;
  }
  LLVM_DEBUG(dbgs() << "*** released Top (ready = " << SU->TopReadyCycle << "): " << *MI);
  if (MI->getOpcode() == TPC::MOVnodce) {
    SU->isScheduleHigh = true;
  }
  Top.releaseNode(SU, SU->TopReadyCycle, isPostRA);
}

void TPCSchedStrategy::releaseBottomNode(SUnit *SU) {
  MachineInstr *MI = SU->getInstr();
  (void)MI;
  if (SU->isScheduled) {
    LLVM_DEBUG(dbgs() << "*** release Bot - already scheduled: " << *MI);
    return;
  }
  assert(SU->getInstr() && "Scheduled SUnit must have instr");

  for (SUnit::succ_iterator I = SU->Succs.begin(), E = SU->Succs.end();
       I != E; ++I) {
    unsigned SuccReadyCycle = I->getSUnit()->BotReadyCycle;
    unsigned MinLatency = I->getLatency();
#ifndef NDEBUG
    Bot.MaxMinLatency = std::max(MinLatency, Bot.MaxMinLatency);
#endif
    if (SU->BotReadyCycle < SuccReadyCycle + MinLatency)
      SU->BotReadyCycle = SuccReadyCycle + MinLatency;
  }
  LLVM_DEBUG(dbgs() << "*** released Bot (ready = " << SU->BotReadyCycle << "): " << *MI);
  Bot.releaseNode(SU, SU->BotReadyCycle, isPostRA);
}

/// If this queue only has one ready candidate, return it. As a side effect,
/// advance the cycle until at least one node is ready. If multiple instructions
/// are ready, return NULL.
SUnit *TPCSchedStrategy::TPCSchedBoundary::pickOnlyChoice() {
  for (unsigned i = 0; Available.empty(); ++i) {
    assert(i <= (HazardRec->getMaxLookAhead() + MaxMinLatency) &&
           "permanent hazard"); (void)i;
    ResourceModel->reserveResources(nullptr);
    bumpCycle();
  }

  // There is only one instruction available from the queue - simply return it.
  if (Available.size() == 1) {
    LLVM_DEBUG(dbgs() << "*** " << getName() << " pickOnlyChoice\n");
    LLVM_DEBUG(dbgs() << "   ");
    LLVM_DEBUG(DAG->dumpNode(**Available.begin()));
    return *Available.begin();
  }
  return nullptr;
}

bool TPCSchedStrategy::tryChangeSlot(SUnit *SU, bool do_change) {
  const TPCInstrInfo * TII = DAG->MF.getSubtarget<TPCSubtarget>().getInstrInfo();
  MachineInstr * MI = SU->getInstr();
  std::vector<unsigned> alt_opcodes;
  bool changed = false;
  unsigned opc_orig = MI->getOpcode();
  if (TII->getOtherSlotOpcodes(MI, alt_opcodes)) {
    for (auto opc : alt_opcodes) {
      // SW-63283 (FCLASS without LIMIT and CALC_FP_SPECIAL with a single source
      // break simulator when scheduled on LOAD slot)
      if (DAG->MF.getSubtarget<TPCSubtarget>().hasDoron1ISA() &&
          TPCVectorOpsOnLd == false &&
          (opc == TPC::LD_CALC_FP_SPECIALOneArgvvp ||
           opc == TPC::LD_CALC_FP_SPECIALOneArgvvm ||
           opc == TPC::LD_FCLASSvvp ||
           opc == TPC::LD_FCLASSvvm ||
           opc == TPC::LD_FCLASSvsp ||
           opc == TPC::LD_FCLASSvsm)) {
        continue;
      }
      MI->setDesc(TII->get(opc));
      if (Top.HazardRec->getHazardType(SU) == ScheduleHazardRecognizer::NoHazard) {
        changed = true;
        break;
      }
    }
    if (!changed) {
      MI->setDesc(TII->get(opc_orig));
    }
  }
  if (changed && !do_change) {
    MI->setDesc(TII->get(opc_orig));
  }
  return changed;
}

TPCSchedStrategy::CandResult TPCSchedStrategy::
pickNodeFromQueue(ReadyQueue &Q, const RegPressureTracker &RPTracker,
                  SchedCandidate &Candidate) {

  // getMaxPressureDelta temporarily modifies the tracker.
  RegPressureTracker &TempTracker = const_cast<RegPressureTracker&>(RPTracker);

  LLVM_DEBUG(
  dbgs() << "*** Instructions available at cycle Top=" << Top.CurrCycle << " Bot=" << Bot.CurrCycle << "\n";  
  for (ReadyQueue::iterator I = Q.begin(), E = Q.end(); I != E; ++I) {
    dbgs() << "   R" << (*I)->TopReadyCycle << " ";
    DAG->dumpNode(**I);
  }
  );

  // BestSU remains NULL if no top candidates beat the best existing candidate.
  CandResult FoundCandidate = NoCand;
  for (ReadyQueue::iterator I = Q.begin(), E = Q.end(); I != E; ++I) {
    RegPressureDelta RPDelta;
    if (!isPostRA) {
      TempTracker.getMaxPressureDelta((*I)->getInstr(), RPDelta,
                                      DAG->getRegionCriticalPSets(),
                                      DAG->getRegPressure().MaxSetPressure);
    }

    int CurrentCost = SchedulingCost(Q, *I, Candidate, RPDelta, false);

  LLVM_DEBUG(dbgs() << "*** Cost(" << CurrentCost << "):   ");  
  LLVM_DEBUG(DAG->dumpNode(**I));

    // Initialize the candidate if needed.
    if (!Candidate.SU) {
      Candidate.SU = *I;
      Candidate.RPDelta = RPDelta;
      Candidate.SCost = CurrentCost;
      FoundCandidate = NodeOrder;
      continue;
    }

    // Best cost.
    if (CurrentCost > Candidate.SCost /*&& !(*I)->getInstr()->isPseudo()*/) {
      Candidate.SU = *I;
      Candidate.RPDelta = RPDelta;
      Candidate.SCost = CurrentCost;
      FoundCandidate = BestCost;
      continue;
    }

#if 0
    if (CurrentCost == Candidate.SCost) {
      unsigned CurrSize, CandSize;
      if (Q.getID() == TopQID) {
        CurrSize = (*I)->Succs.size();
        CandSize = Candidate.SU->Succs.size();
      } else {
        CurrSize = (*I)->Preds.size();
        CandSize = Candidate.SU->Preds.size();
      }
      if (CurrSize > CandSize) {
        Candidate.SU = *I;
        Candidate.RPDelta = RPDelta;
        Candidate.SCost = CurrentCost;
        FoundCandidate = BestCost;
      }
      // Keep the old candidate if it's a better candidate. That is, don't use
      // the subsequent tie breaker.
      if (CurrSize != CandSize)
        continue;
    }
#endif

    // To avoid scheduling indeterminism, we need a tie breaker
    // for the case when cost is identical for two nodes.
    //
    if (CurrentCost == Candidate.SCost) {
      if ((Q.getID() == TopQID && (*I)->NodeNum < Candidate.SU->NodeNum)
          || (Q.getID() == BotQID && (*I)->NodeNum > Candidate.SU->NodeNum)) {
        Candidate.SU = *I;
        Candidate.RPDelta = RPDelta;
        Candidate.SCost = CurrentCost;
        FoundCandidate = NodeOrder;
        continue;
      }
    }

    // Fall through to original instruction order.
    // Only consider node order if Candidate was chosen from this Q.
    if (FoundCandidate == NoCand) {
      LLVM_DEBUG(dbgs() << " *** No candidate found\n");
      continue;
    }
  }

  if (Top.HazardRec->isEnabled()) {
    if (Top.HazardRec->getHazardType(Candidate.SU) != ScheduleHazardRecognizer::NoHazard) {
      tryChangeSlot(Candidate.SU, true);
    }
  }

  LLVM_DEBUG(dbgs() << "*** Selected (" << Candidate.SCost << ")\n");  
  LLVM_DEBUG(dbgs() << "   ");
  //LLVM_DEBUG(Candidate.SU->dumpAttributes());
  LLVM_DEBUG(DAG->dumpNode(*(Candidate.SU)));

#if 0
  LLVM_DEBUG(if (Candidate.SU->getInstr() && isHWRegProducer(*Candidate.SU->getInstr())) {
    dbgs() << "*** HWReg Producer\n";
  });
#endif

  return FoundCandidate;
}

/// isSingleUnscheduledPred - If SU2 is the only unscheduled predecessor
/// of SU, return true (we may have duplicates)
///
static inline bool isSingleUnscheduledPred(SUnit *SU, SUnit *SU2) {
  if (SU->NumPredsLeft == 0)
    return false;

  for (auto &Pred : SU->Preds) {
    // We found an available, but not scheduled, predecessor.
    if (!Pred.getSUnit()->isScheduled && (Pred.getSUnit() != SU2))
      return false;
  }

  return true;
}

/// isSingleUnscheduledSucc - If SU2 is the only unscheduled successor
/// of SU, return true (we may have duplicates)
///
static inline bool isSingleUnscheduledSucc(SUnit *SU, SUnit *SU2) {
  if (SU->NumSuccsLeft == 0)
    return false;

  for (auto &Succ : SU->Succs) {
    // We found an available, but not scheduled, successor.
    if (!Succ.getSUnit()->isScheduled && (Succ.getSUnit() != SU2))
      return false;
  }
  return true;
}

/// Check if the instruction changes the register pressure of a register in the
/// high pressure set. The function returns a negative value if the pressure
/// decreases and a positive value is the pressure increases. If the instruction
/// doesn't use a high pressure register or doesn't change the register
/// pressure, then return 0.
int TPCSchedStrategy::pressureChange(const SUnit *SU, bool isBotUp) {
  PressureDiff &PD = DAG->getPressureDiff(SU);
  for (auto &P : PD) {
    if (!P.isValid())
      continue;
    // The pressure differences are computed bottom-up, so the comparision for
    // an increase is positive in the bottom direction, but negative in the
    //  top-down direction.
    if (HighPressureSets[P.getPSet()]) {
      //dbgs() << "pressureChange: " << (-P.getUnitInc()) << ": ";
      //SU->dump(DAG);
      return (isBotUp ? P.getUnitInc() : -P.getUnitInc());
    }
  }
  return 0;
}


// Constants used to denote relative importance of
// heuristic components for cost computation.

/// Bonus assigned to scheduling candidate if it does not have scheduling hazards.
static const int BonusNoHazards = 325;

/// Bouns for scheduling candidate for which resources are available. It is used
/// when hazard recognizer is disabled.
static const int BonusForAvailableResources = 125;

/// Bonus assigned to scheduling candidate for each blocked unit.
static const int BonusForBlockedUnit = 10;

/// Bonus for unit height.
static const int BonusForUnitHeight = 10;

/// Bonus for instructions that have high scheduling prioritity.
static const int BonusForHighPriority = 200;

/// Bonus for the COPY, which is the first instruction in the current packet.
static const int BonusForTheFirstCopy = 200;

/// Bonus for the COPY, which is the second instruction in the current packet.
static const int BonusForTheSecondCopy = 50;

/// Bonus for the COPY, which is the third instruction in the current packet.
static const int BonusForTheThirdCopy = 75;

/// Penalty for each cycle of latency.
static const int PenaltyForLatencyCycle = 1000;

/// Penalty for each unit of register pressure Exceed.
static const int PenaltyRPExceed = 800;

/// Penalty for each unit of register pressure CriticalMax.
static const int PenaltyRPCriticalMax = 200;

/// Penalty for each unit of register pressure CurrentMax.
static const int PenaltyRPCurrentMax = 50;

/// Penalty for ADRF definition if number of ADRFs exceeds limit.
static const int PenaltyADRFUse = 200000;

/// Penalty For LOOKUP.
static const int PenaltyForLookup = 200;

/// Penalty For SET_INDX.
static const int PenaltyForSetIndex = 50;

/// Penalty for unspillable reg definition if number of regs exceeds limit.
static const int PenaltyNonSpillable = 4000;


/// Single point to compute overall scheduling cost.
/// TODO: add more heuristics.
int TPCSchedStrategy::SchedulingCost(ReadyQueue &Q, SUnit *SU,
                                     SchedCandidate &Candidate,
                                     RegPressureDelta &Delta,
                                     bool verbose) {
  // Do not waste time on a node that is already scheduled.
  if (!SU || SU->isScheduled)
    return 1;

  int Bonus = 0;
  int Penalty = 0;

  MachineInstr &Instr = *SU->getInstr();
  LLVM_DEBUG(dbgs() << ((Q.getID() == TopQID) ? "(top|" : "(bot|"));

  // Forced priority is high.
  if (SU->isScheduleHigh) {
    Bonus += BonusForHighPriority;
    LLVM_DEBUG(dbgs() << "High | ");
  }

  // Do not allow any instruction in the loop block to be scheduled
  // earlier then LOOP instruction.
  if (TPCII::isLoopInst(Instr.getDesc())) {
    Bonus += BonusForHighPriority;
    LLVM_DEBUG(dbgs() << "Loop | ");
  }

  // Check if the SU to be scheduled has latency hazard (i.e. not ready to be scheduled).
  //
  int latencyHazard = 0;
  if (Q.getID() == TopQID) {
    for (const SDep &PI : SU->Preds) {
      unsigned PredReadyCycle = PI.getSUnit()->TopReadyCycle;
      unsigned MinLatency = PI.getLatency();
      if (Top.CurrCycle < PredReadyCycle + MinLatency) {
        int lat = (PredReadyCycle + MinLatency) - Top.CurrCycle;
        if (latencyHazard < lat) {
          latencyHazard = lat;
        }
      }
    }
  }
  bool schedForMinRegs = false;
  if (!isPostRA) {
    schedForMinRegs = DAG->ScheduleForMinRegs;
  }
  if (latencyHazard > 0 && !schedForMinRegs) {
    LLVM_DEBUG(dbgs() << "Lat(" << latencyHazard << ") | ");
    Penalty += latencyHazard * PenaltyForLatencyCycle;
  }

  int ResCount = 1;

  if (Q.getID() == TopQID) {
    if (Top.isLatencyBound(SU) || isPostRA) {
      Bonus += (SU->getHeight() * BonusForUnitHeight);
      LLVM_DEBUG(dbgs() << "HLB(" << SU->getHeight() << ") | ");
    } else {
      LLVM_DEBUG(dbgs() << "H(" << SU->getHeight() << ") | ");
    }

    // Change slot for IRF copy
    if (Instr.getOpcode() == TPC::MOVIIp) {
      tryChangeSlot(SU, true);
    }

    // If resources are available for it, multiply the chance of scheduling.
    if (latencyHazard == 0) {
      if (Top.HazardRec->isEnabled()) {
        bool HazardResolved = (Top.HazardRec->getHazardType(SU) == ScheduleHazardRecognizer::NoHazard);
        if (!HazardResolved) {
          if (tryChangeSlot(SU, false)) {
            LLVM_DEBUG(dbgs() << "BR | ");
            HazardResolved = true;
          }
        }
        if (HazardResolved) {
          // We have to do something with pseudo instructions, such as COPY - we
          // do not know whether such instruction can be inserted in current VLIW
          // instruction or not. Do some guess here.
          //
          if (Instr.isCopy()) {
            unsigned sz = Top.ResourceModel->getCurPacketSize();
            switch (sz) {
            case 0: Bonus += BonusForTheFirstCopy; break;
            case 1: Bonus += BonusForTheSecondCopy; break;
            case 2: Bonus += BonusForTheThirdCopy; break;
            default: break;
            }
          } else {
            Bonus += BonusNoHazards;
          }
          LLVM_DEBUG(dbgs() << "B|");
        }
      } else if (Top.ResourceModel->isResourceAvailable(SU)) {
        Bonus += BonusForAvailableResources;
      }
    }
  } else { // (Q.getID() == BotQID)
    if (Bot.isLatencyBound(SU)) {
      Bonus += BonusNoHazards;
      LLVM_DEBUG(dbgs() << "DLB(" << SU->getDepth() << ")|");
    }

    // If resources are available for it, multiply the chance of scheduling.
    if (latencyHazard == 0) {
      if (Bot.HazardRec->isEnabled()) {
        bool HazardResolved = (Top.HazardRec->getHazardType(SU) == ScheduleHazardRecognizer::NoHazard);
        if (!HazardResolved) {
          if (tryChangeSlot(SU, false)) {
            Bonus += BonusNoHazards;
            LLVM_DEBUG(dbgs() << "BR|");
          }
        }
        if (HazardResolved) {
          Bonus += BonusNoHazards;
          LLVM_DEBUG(dbgs() << "B|");
        }
      } else if (Bot.ResourceModel->isResourceAvailable(SU)) {
        Bonus += BonusForAvailableResources;
      }
    }
  }

  int NumNodesBlocking = 0;
  if (Q.getID() == TopQID) {
    // How many SUs does it block from scheduling?
    // Look at all of the successors of this node.
    // Count the number of nodes that
    // this node is the sole unscheduled node for.
    if (Top.isLatencyBound(SU) || isPostRA) {
      for (const SDep &SI : SU->Succs)
        if (isSingleUnscheduledPred(SI.getSUnit(), SU))
          ++NumNodesBlocking;
    }
  } else {
    // How many unscheduled predecessors block this node?
    if (Bot.isLatencyBound(SU)) {
      for (const SDep &PI : SU->Preds)
        if (isSingleUnscheduledSucc(PI.getSUnit(), SU))
          ++NumNodesBlocking;
    }
  }
  LLVM_DEBUG(dbgs() << "NB(" << NumNodesBlocking << ") | ");
  Bonus += (NumNodesBlocking * BonusForBlockedUnit);

  if (!isPostRA && !schedForMinRegs) {
    // Less preference to SET_INDX to be able to try another slot for it
    if (Instr.getOpcode() == TPC::SET_INDX_ld_rp || Instr.getOpcode() == TPC::SET_INDX_ld_ip) {
      Penalty += PenaltyForSetIndex;
    }
  }
    // Less preference to LOOKUP to be able to try another slot for it
    if (TPCII::isLookupC(Instr.getDesc()) && !schedForMinRegs) {
      Penalty += PenaltyForLookup;
    }

  // Factor in reg pressure as a heuristic.
  //
  if (!isPostRA && !TPCIgnoreBBRegPressure) {
    LLVM_DEBUG(
        dbgs() << "RP(" << Delta.Excess.getUnitInc() << "/"
               << Delta.CriticalMax.getUnitInc() <<"/"
               << Delta.CurrentMax.getUnitInc() << ")|";
    );
    Penalty += (Delta.Excess.getUnitInc() * PenaltyRPExceed);
    Penalty += (Delta.CriticalMax.getUnitInc() * PenaltyRPCriticalMax);
    Penalty += (Delta.CurrentMax.getUnitInc()  * PenaltyRPCurrentMax);

    const TPCInstrInfo * TII = DAG->MF.getSubtarget<TPCSubtarget>().getInstrInfo();
    if (Delta.Excess.getUnitInc() > 0 && TII->instrProducesUnspillableReg(Instr)) {
      // If scheduling the instruction causes register spilling
      // but the register it produces is non spillable then increse
      // the penalty to be big enough
      Penalty += PenaltyNonSpillable;
    }
  }

  if (!isPostRA && isDefADRF(SU)) {
   // unsigned rp = Top.NumADRFregs;
    unsigned rp = 1 + getRegPressure(SU, &TPC::ADRFRegClass);
    LLVM_DEBUG(dbgs() << "ADRF(" << rp << ") | ");
    if (rp > 6) {
      Penalty += PenaltyADRFUse;
    }
  }

  LLVM_DEBUG(dbgs() << "\n");
  return ResCount + Bonus - Penalty;
}

/// Remove SU from the ready set for this boundary.
void TPCSchedStrategy::TPCSchedBoundary::removeReady(SUnit *SU) {
  if (Available.isInQueue(SU))
    Available.remove(Available.find(SU));
}

void TPCSchedStrategy::TPCSchedBoundary::releaseNode(SUnit *SU,
                                                     unsigned ReadyCycle,
						     bool isPostRA) {
  //LLVM_DEBUG(dbgs() << "TPCSchedStrategy :: TPCSchedBoundary :: releaseNode\n");
  if (ReadyCycle < MinReadyCycle)
    MinReadyCycle = ReadyCycle;
  Available.push(SU);
}


/// Move the boundary of scheduled code by one cycle.
void TPCSchedStrategy::TPCSchedBoundary::bumpCycle() {
  LLVM_DEBUG(dbgs() << "*** " << getName() << " bumpCycle \n";);
  unsigned Width = SchedModel->getIssueWidth();
  IssueCount = (IssueCount <= Width) ? 0 : IssueCount - Width;

  assert(MinReadyCycle < UINT_MAX && "MinReadyCycle uninitialized");
  unsigned NextCycle = std::max(CurrCycle + 1, MinReadyCycle);
  LLVM_DEBUG(dbgs() << "  NextCycle=" << NextCycle << "\n";);

  if (!HazardRec->isEnabled()) {
    // Bypass HazardRec virtual calls.
    CurrCycle = NextCycle;
  } else {
    // Bypass getHazardType calls in case of long latency.
    for (; CurrCycle != NextCycle; ++CurrCycle) {
      if (isTop())
        HazardRec->AdvanceCycle();
      else
        HazardRec->RecedeCycle();
    }
  }
  LLVM_DEBUG(dbgs() << "*** " << getName() << " Next cycle " << CurrCycle << '\n');
}

void TPCSchedStrategy::TPCSchedBoundary::bumpNode(SUnit *SU, bool isPostRA) {
  bool startNewCycle = false;

  MachineInstr *MI = SU->getInstr();
  (void)MI;
  LLVM_DEBUG(dbgs() << "*** " << getName() << " bumpNode: " << *MI);

  // Update the reservation table.
  if (HazardRec->isEnabled()) {
    if (HazardRec->getHazardType(SU) != ScheduleHazardRecognizer::NoHazard) {
        const TPCInstrInfo * TII = DAG->MF.getSubtarget<TPCSubtarget>().getInstrInfo();
        std::vector<unsigned> alt_opcodes;
        if (TII->getOtherSlotOpcodes(SU->getInstr(), alt_opcodes)) {
	   unsigned opc_orig = SU->getInstr()->getOpcode();
           bool changed = false;
	   for (auto opc : alt_opcodes) {
             // SW-63283 (FCLASS without LIMIT and CALC_FP_SPECIAL with a single source
             // break simulator when scheduled on LOAD slot)
             if (DAG->MF.getSubtarget<TPCSubtarget>().hasDoron1ISA() &&
                 TPCVectorOpsOnLd == false &&
                 (opc == TPC::LD_CALC_FP_SPECIALOneArgvvp ||
                  opc == TPC::LD_CALC_FP_SPECIALOneArgvvm ||
                  opc == TPC::LD_FCLASSvvp ||
                  opc == TPC::LD_FCLASSvvm ||
                  opc == TPC::LD_FCLASSvsp ||
                  opc == TPC::LD_FCLASSvsm)) {
               continue;
             }
             SU->getInstr()->setDesc(TII->get(opc));
             if (HazardRec->getHazardType(SU) == ScheduleHazardRecognizer::NoHazard) {
	       changed = true;
	       break;
             }
	   }
	   if (!changed) {
             SU->getInstr()->setDesc(TII->get(opc_orig));
             startNewCycle = true;
	   }
	}
	else {
          startNewCycle = true;
	}
    }
    else {
      if (SU->TopReadyCycle > CurrCycle) {
          startNewCycle = true;
	  MinReadyCycle = SU->TopReadyCycle;
      }
      if (SU->getInstr()->isPseudo() && SU->getInstr()->isCopy() &&
          ResourceModel->getCurPacketSize() >= 2) {
          startNewCycle = true;
      }
    }
  }

  // Check the instruction group dispatch limit.
  // TODO: Check if this SU must end a dispatch group.
  IssueCount += SchedModel->getNumMicroOps(SU->getInstr());
  if (startNewCycle) {
    LLVM_DEBUG(dbgs() << "*** Starting new instr at cycle " << CurrCycle << '\n');
    bumpCycle();
  }
  else {
    LLVM_DEBUG(dbgs() << "*** " << getName() << " IssueCount " << IssueCount
          << " at cycle " << CurrCycle << '\n');
  }
  if (HazardRec->isEnabled()) {
    HazardRec->EmitInstruction(SU);
  }
}


namespace llvm {

ScheduleDAGInstrs *createTPCMachineScheduler(MachineSchedContext *C) {
  return new TPCMachineScheduler(C, std::make_unique<TPCSchedStrategy>());
}

ScheduleDAGInstrs *createTPCPostMachineScheduler(MachineSchedContext *C) {
  //return new TPCMachineScheduler(C, llvm::make_unique<TPCSchedStrategy>());
  //return new ScheduleDAGMI(C, make_unique<PostGenericScheduler>(C), /*RemoveKillFlags=*/true);
  ScheduleDAGMI* DAG = new ScheduleDAGMI(C, std::make_unique<TPCPostRASchedStrategy>(), /*RemoveKillFlags=*/true);
  DAG->addMutation(std::make_unique<TPCSubtarget::TPCDAGMutation>());
  return DAG;
}

// Register tpc scheduler so that it can be specified in command line.
static MachineSchedRegistry
TPCSchedulerRegistry("tpc", "Run TPC's custom scheduler",
                     createTPCMachineScheduler);

}

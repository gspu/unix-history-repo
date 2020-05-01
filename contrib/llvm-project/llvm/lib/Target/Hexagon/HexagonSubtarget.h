//===- HexagonSubtarget.h - Define Subtarget for the Hexagon ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Hexagon specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONSUBTARGET_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONSUBTARGET_H

#include "HexagonDepArch.h"
#include "HexagonFrameLowering.h"
#include "HexagonISelLowering.h"
#include "HexagonInstrInfo.h"
#include "HexagonRegisterInfo.h"
#include "HexagonSelectionDAGInfo.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/ScheduleDAGMutation.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrItineraries.h"
#include <memory>
#include <string>
#include <vector>

#define GET_SUBTARGETINFO_HEADER
#include "HexagonGenSubtargetInfo.inc"

namespace llvm {

class MachineInstr;
class SDep;
class SUnit;
class TargetMachine;
class Triple;

class HexagonSubtarget : public HexagonGenSubtargetInfo {
  virtual void anchor();

  bool UseHVX64BOps = false;
  bool UseHVX128BOps = false;

  bool UseLongCalls = false;
  bool UseMemops = false;
  bool UsePackets = false;
  bool UseNewValueJumps = false;
  bool UseNewValueStores = false;
  bool UseSmallData = false;
  bool UseZRegOps = false;

  bool HasMemNoShuf = false;
  bool EnableDuplex = false;
  bool ReservedR19 = false;
  bool NoreturnStackElim = false;

public:
  Hexagon::ArchEnum HexagonArchVersion;
  Hexagon::ArchEnum HexagonHVXVersion = Hexagon::ArchEnum::NoArch;
  CodeGenOpt::Level OptLevel;
  /// True if the target should use Back-Skip-Back scheduling. This is the
  /// default for V60.
  bool UseBSBScheduling;

  struct UsrOverflowMutation : public ScheduleDAGMutation {
    void apply(ScheduleDAGInstrs *DAG) override;
  };
  struct HVXMemLatencyMutation : public ScheduleDAGMutation {
    void apply(ScheduleDAGInstrs *DAG) override;
  };
  struct CallMutation : public ScheduleDAGMutation {
    void apply(ScheduleDAGInstrs *DAG) override;
  private:
    bool shouldTFRICallBind(const HexagonInstrInfo &HII,
          const SUnit &Inst1, const SUnit &Inst2) const;
  };
  struct BankConflictMutation : public ScheduleDAGMutation {
    void apply(ScheduleDAGInstrs *DAG) override;
  };

private:
  std::string CPUString;
  HexagonInstrInfo InstrInfo;
  HexagonRegisterInfo RegInfo;
  HexagonTargetLowering TLInfo;
  HexagonSelectionDAGInfo TSInfo;
  HexagonFrameLowering FrameLowering;
  InstrItineraryData InstrItins;

public:
  HexagonSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                   const TargetMachine &TM);

  /// getInstrItins - Return the instruction itineraries based on subtarget
  /// selection.
  const InstrItineraryData *getInstrItineraryData() const override {
    return &InstrItins;
  }
  const HexagonInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const HexagonRegisterInfo *getRegisterInfo() const override {
    return &RegInfo;
  }
  const HexagonTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const HexagonFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const HexagonSelectionDAGInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  HexagonSubtarget &initializeSubtargetDependencies(StringRef CPU,
                                                    StringRef FS);

  /// ParseSubtargetFeatures - Parses features string setting specified
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  bool hasV5Ops() const {
    return getHexagonArchVersion() >= Hexagon::ArchEnum::V5;
  }
  bool hasV5OpsOnly() const {
    return getHexagonArchVersion() == Hexagon::ArchEnum::V5;
  }
  bool hasV55Ops() const {
    return getHexagonArchVersion() >= Hexagon::ArchEnum::V55;
  }
  bool hasV55OpsOnly() const {
    return getHexagonArchVersion() == Hexagon::ArchEnum::V55;
  }
  bool hasV60Ops() const {
    return getHexagonArchVersion() >= Hexagon::ArchEnum::V60;
  }
  bool hasV60OpsOnly() const {
    return getHexagonArchVersion() == Hexagon::ArchEnum::V60;
  }
  bool hasV62Ops() const {
    return getHexagonArchVersion() >= Hexagon::ArchEnum::V62;
  }
  bool hasV62OpsOnly() const {
    return getHexagonArchVersion() == Hexagon::ArchEnum::V62;
  }
  bool hasV65Ops() const {
    return getHexagonArchVersion() >= Hexagon::ArchEnum::V65;
  }
  bool hasV65OpsOnly() const {
    return getHexagonArchVersion() == Hexagon::ArchEnum::V65;
  }
  bool hasV66Ops() const {
    return getHexagonArchVersion() >= Hexagon::ArchEnum::V66;
  }
  bool hasV66OpsOnly() const {
    return getHexagonArchVersion() == Hexagon::ArchEnum::V66;
  }

  bool useLongCalls() const { return UseLongCalls; }
  bool useMemops() const { return UseMemops; }
  bool usePackets() const { return UsePackets; }
  bool useNewValueJumps() const { return UseNewValueJumps; }
  bool useNewValueStores() const { return UseNewValueStores; }
  bool useSmallData() const { return UseSmallData; }
  bool useZRegOps() const { return UseZRegOps; }

  bool useHVXOps() const {
    return HexagonHVXVersion > Hexagon::ArchEnum::NoArch;
  }
  bool useHVX128BOps() const { return useHVXOps() && UseHVX128BOps; }
  bool useHVX64BOps() const { return useHVXOps() && UseHVX64BOps; }

  bool hasMemNoShuf() const { return HasMemNoShuf; }
  bool hasReservedR19() const { return ReservedR19; }
  bool usePredicatedCalls() const;

  bool noreturnStackElim() const { return NoreturnStackElim; }

  bool useBSBScheduling() const { return UseBSBScheduling; }
  bool enableMachineScheduler() const override;

  // Always use the TargetLowering default scheduler.
  // FIXME: This will use the vliw scheduler which is probably just hurting
  // compiler time and will be removed eventually anyway.
  bool enableMachineSchedDefaultSched() const override { return false; }

  AntiDepBreakMode getAntiDepBreakMode() const override { return ANTIDEP_ALL; }
  bool enablePostRAScheduler() const override { return true; }

  bool enableSubRegLiveness() const override;

  const std::string &getCPUString () const { return CPUString; }

  const Hexagon::ArchEnum &getHexagonArchVersion() const {
    return HexagonArchVersion;
  }

  void getPostRAMutations(
      std::vector<std::unique_ptr<ScheduleDAGMutation>> &Mutations)
      const override;

  void getSMSMutations(
      std::vector<std::unique_ptr<ScheduleDAGMutation>> &Mutations)
      const override;

  /// Enable use of alias analysis during code generation (during MI
  /// scheduling, DAGCombine, etc.).
  bool useAA() const override;

  /// Perform target specific adjustments to the latency of a schedule
  /// dependency.
  void adjustSchedDependency(SUnit *def, SUnit *use, SDep& dep) const override;

  unsigned getVectorLength() const {
    assert(useHVXOps());
    if (useHVX64BOps())
      return 64;
    if (useHVX128BOps())
      return 128;
    llvm_unreachable("Invalid HVX vector length settings");
  }

  ArrayRef<MVT> getHVXElementTypes() const {
    static MVT Types[] = { MVT::i8, MVT::i16, MVT::i32 };
    return makeArrayRef(Types);
  }

  bool isHVXVectorType(MVT VecTy, bool IncludeBool = false) const {
    if (!VecTy.isVector() || !useHVXOps() || VecTy.isScalableVector())
      return false;
    MVT ElemTy = VecTy.getVectorElementType();
    if (!IncludeBool && ElemTy == MVT::i1)
      return false;

    unsigned HwLen = getVectorLength();
    unsigned NumElems = VecTy.getVectorNumElements();
    ArrayRef<MVT> ElemTypes = getHVXElementTypes();

    if (IncludeBool && ElemTy == MVT::i1) {
      // Special case for the v512i1, etc.
      if (8*HwLen == NumElems)
        return true;
      // Boolean HVX vector types are formed from regular HVX vector types
      // by replacing the element type with i1.
      for (MVT T : ElemTypes)
        if (NumElems * T.getSizeInBits() == 8*HwLen)
          return true;
      return false;
    }

    unsigned VecWidth = VecTy.getSizeInBits();
    if (VecWidth != 8*HwLen && VecWidth != 16*HwLen)
      return false;
    return llvm::any_of(ElemTypes, [ElemTy] (MVT T) { return ElemTy == T; });
  }

  unsigned getTypeAlignment(MVT Ty) const {
    if (isHVXVectorType(Ty, true))
      return getVectorLength();
    return Ty.getSizeInBits() / 8;
  }

  unsigned getL1CacheLineSize() const;
  unsigned getL1PrefetchDistance() const;

private:
  // Helper function responsible for increasing the latency only.
  void updateLatency(MachineInstr &SrcInst, MachineInstr &DstInst, SDep &Dep)
      const;
  void restoreLatency(SUnit *Src, SUnit *Dst) const;
  void changeLatency(SUnit *Src, SUnit *Dst, unsigned Lat) const;
  bool isBestZeroLatency(SUnit *Src, SUnit *Dst, const HexagonInstrInfo *TII,
      SmallSet<SUnit*, 4> &ExclSrc, SmallSet<SUnit*, 4> &ExclDst) const;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONSUBTARGET_H

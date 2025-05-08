// Implementation adopted from javidx9 NES series: https://youtu.be/AAcEk5pWXPY?si=HHJ8Tqm3iVRwWf9O

#pragma once
#include <array>
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper4 : public Mapper
{
public:
  Mapper4( iNes2Instance iNesHeader ) : Mapper( iNesHeader ) { Reset(); }
  auto MapCpuAddr( u16 address ) -> u32 override;
  auto MapPpuAddr( u16 address ) -> u32 override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  bool SupportsPrgRam() override { return true; }
  bool HasExpansionRom() override { return false; }
  bool HasExpansionRam() override { return false; }

  MirrorMode GetMirrorMode() override { return mirroring; }

  // ---
  void Reset() override;
  bool IsIrqRequested() override { return bIsIrqRequested; }
  void IrqClear() override { bIsIrqRequested = false; }
  void CountScanline() override
  {
    if ( nIrqCounter == 0 ) {
      nIrqCounter = nIrqReload;
    } else {
      nIrqCounter--;
    }

    if ( nIrqCounter == 0 && bIrqEnabled ) {
      bIsIrqRequested = true;
    }
  }

  u8   nTargetRegister = 0x00;
  bool bPrgBankMode = false;
  bool bChrInversion = false;

  std::array<u32, 8> pRegister{};
  std::array<u32, 8> pChrBank{};
  std::array<u32, 4> pPrgBank{};

  bool bIsIrqRequested = false;
  bool bIrqEnabled = false;
  u16  nIrqCounter = 0x0000;
  u16  nIrqReload = 0x0000;
};

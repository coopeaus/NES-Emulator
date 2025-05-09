#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper1 : public Mapper
{

public:
  Mapper1( iNes2Instance iNesHeader ) : Mapper( iNesHeader ) { Reset(); }
  auto MapCpuAddr( u16 address ) -> u32 override;
  auto MapPpuAddr( u16 address ) -> u32 override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  bool       SupportsPrgRam() override { return true; }
  bool       HasExpansionRom() override { return false; }
  bool       HasExpansionRam() override { return false; }
  MirrorMode GetMirrorMode() override;

  bool IsIrqRequested() override { return false; }
  void IrqClear() override {}
  void CountScanline() override {}

  void Reset() override
  {
    controlRegister = 0x1C;
    shiftRegister = 0x10;
    writeCount = 0;
    prgBank16Lo = 0;
    prgBank16Hi = GetPrgBankCount() - 1;
    prgBank32 = 0;
    chrBank4Lo = 0;
    chrBank4Hi = 0;
    chrBank8 = 0;
    mirroring = MirrorMode::SingleLower;
  }

  u8 controlRegister{ 0x1C };

  // PRG bank selectors.
  u8 prgBank16Lo{ 0 };
  u8 prgBank16Hi{};
  u8 prgBank32{ 0 };

  // CHR bank selectors
  u8 chrBank4Lo{ 0 };
  u8 chrBank4Hi{ 0 };
  u8 chrBank8{ 0 };

  // Serial loading mechanism
  u8 shiftRegister{ 0x10 };
  u8 writeCount{ 0 };
};

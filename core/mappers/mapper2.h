#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper2 : public Mapper
{

public:
  Mapper2( iNes2Instance iNes2Header ) : Mapper( iNes2Header ) { Reset(); }
  auto MapCpuAddr( u16 address ) -> u32 override;
  auto MapPpuAddr( u16 address ) -> u32 override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  bool       SupportsPrgRam() override { return iNes.GetBatteryMode(); }
  bool       HasExpansionRom() override { return false; }
  bool       HasExpansionRam() override { return false; }
  MirrorMode GetMirrorMode() override;

  bool IsIrqRequested() override { return false; }
  void IrqClear() override {}
  void CountScanline() override {}
  void Reset() override
  {
    prgBank16Lo = 0;
    mirroring = MirrorMode::Vertical;
  }

  u8         prgBank16Lo{ 0 };
  MirrorMode mirroring = MirrorMode::Vertical;
};

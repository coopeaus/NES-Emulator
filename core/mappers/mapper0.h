#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper0 : public Mapper
{
public:
  Mapper0( iNes2Instance iNesHeader ) : Mapper( iNesHeader ) {}
  auto MapCpuAddr( u16 address ) -> u32 override;
  auto MapPpuAddr( u16 address ) -> u32 override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  void Reset() override {}
  bool SupportsPrgRam() override { return iNes.GetBatteryMode(); }
  bool HasExpansionRom() override { return false; }
  bool HasExpansionRam() override { return false; }

  bool IsIrqRequested() override { return false; }
  void IrqClear() override {}
  void CountScanline() override {}

  MirrorMode GetMirrorMode() override { return mirroring; }
};

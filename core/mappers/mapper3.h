#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

/**
 * @brief Mapper 3 (CNROM) implementation
 * Supports simple CHR-ROM bank switching via CPU writes to $8000-$FFFF
 */
class Mapper3 : public Mapper
{
public:
  Mapper3( iNes2Instance iNesHeader ) : Mapper( iNesHeader ) { Reset(); }
  u32  MapCpuAddr( u16 address ) override;
  u32  MapPpuAddr( u16 address ) override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  bool       SupportsPrgRam() override { return false; }
  bool       HasExpansionRom() override { return false; }
  bool       HasExpansionRam() override { return false; }
  MirrorMode GetMirrorMode() override;

  bool IsIrqRequested() override { return false; }
  void IrqClear() override {}
  void CountScanline() override {}
  void Reset() override { chrBank = 0; }

  u8 chrBank = 0;
};

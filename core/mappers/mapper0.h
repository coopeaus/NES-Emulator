#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper0 : public Mapper
{
public:
  Mapper0( iNes2Instance iNesHeader ) : Mapper( iNesHeader ) {}
  auto TranslateCPUAddress( u16 address ) -> u32 override;
  auto TranslatePPUAddress( u16 address ) -> u32 override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  [[nodiscard]] bool SupportsPrgRam() override { return false; }
  [[nodiscard]] bool HasExpansionRom() override { return false; }
  [[nodiscard]] bool HasExpansionRam() override { return false; }

  [[nodiscard]] MirrorMode GetMirrorMode() override { return mirroring; }
};

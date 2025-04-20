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
  Mapper3( iNes2Instance iNesHeader );
  u32  TranslateCPUAddress( u16 address ) override;
  u32  TranslatePPUAddress( u16 address ) override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  [[nodiscard]] bool       SupportsPrgRam() override { return false; }
  [[nodiscard]] bool       HasExpansionRom() override { return false; }
  [[nodiscard]] bool       HasExpansionRam() override { return false; }
  [[nodiscard]] MirrorMode GetMirrorMode() override;

private:
  u8 _chrBank = 0;
};

#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper1 : public Mapper
{

public:
  Mapper1( iNes2Instance iNesHeader ) : Mapper( iNesHeader ), prgBank16Hi( GetPrgBankCount() - 1 ) { Reset(); }
  auto MapPrgOffset( u16 address ) -> u32 override;
  auto MapChrOffset( u16 address ) -> u32 override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  [[nodiscard]] bool       SupportsPrgRam() override { return true; }
  [[nodiscard]] bool       HasExpansionRom() override { return false; }
  [[nodiscard]] bool       HasExpansionRam() override { return false; }
  [[nodiscard]] MirrorMode GetMirrorMode() override;

  void Reset();

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

  // Mirroring
  MirrorMode mirrorMode{ MirrorMode::SingleLower };
};

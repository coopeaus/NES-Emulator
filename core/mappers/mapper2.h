#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper2 : public Mapper
{

public:
  Mapper2( iNes2Instance iNes2Header ) : Mapper( iNes2Header ) {}
  auto MapPrgOffset( u16 address ) -> u32 override;
  auto MapChrOffset( u16 address ) -> u32 override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  [[nodiscard]] bool SupportsPrgRam() override { return false; }
  [[nodiscard]] bool HasExpansionRom() override { return false; }
  [[nodiscard]] bool HasExpansionRam() override { return false; }

  [[nodiscard]] MirrorMode GetMirrorMode() override;

private:
  u8         _prgBank16Lo{ 0 };
  MirrorMode _mirrorMode = MirrorMode::Vertical;
};

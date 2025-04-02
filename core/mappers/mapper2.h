#pragma once
#include "cartridge-header.h"
#include "mapper-base.h"

class Mapper2 : public Mapper
{

  public:
    Mapper2( iNes2Instance iNes2Header, MirrorMode mirrorMode )
        : Mapper( iNes2Header ), _mirrorMode( mirrorMode )
    {
    }
    auto TranslateCPUAddress( u16 address ) -> u32 override;
    auto TranslatePPUAddress( u16 address ) -> u32 override;
    void HandleCPUWrite( u16 address, u8 data ) override;

    [[nodiscard]] bool SupportsPrgRam() override { return false; }
    [[nodiscard]] bool HasExpansionRom() override { return false; }
    [[nodiscard]] bool HasExpansionRam() override { return false; }

    [[nodiscard]] MirrorMode GetMirrorMode() override;

  private:
    u8 _prgBank16Lo{ 0 };

    // Mirroring mode (fixed)
    MirrorMode _mirrorMode;
};

#pragma once
#include "mapper-base.h"

class Mapper1 : public Mapper
{

  public:
    Mapper1( iNes2Instance iNesHeader ) : Mapper( iNesHeader ), _prgBank16Hi( GetPrgBankCount() - 1 ) {}
    auto TranslateCPUAddress( u16 address ) -> u32 override;
    auto TranslatePPUAddress( u16 address ) -> u32 override;
    void HandleCPUWrite( u16 address, u8 data ) override;

    [[nodiscard]] bool SupportsPrgRam() override { return true; }
    [[nodiscard]] bool HasExpansionRom() override { return false; }
    [[nodiscard]] bool HasExpansionRam() override { return false; }

    [[nodiscard]] MirrorMode GetMirrorMode() override;

  private:
    u8 _controlRegister{ 0x1C };

    // PRG bank selectors.
    u8 _prgBank16Lo{ 0 };
    u8 _prgBank16Hi{};
    u8 _prgBank32{ 0 };

    // CHR bank selectors
    u8 _chrBank4Lo{ 0 };
    u8 _chrBank4Hi{ 0 };
    u8 _chrBank8{ 0 };

    // Serial loading mechanism
    u8 _shiftRegister{ 0x10 };
    u8 _writeCount{ 0 };

    // Mirroring
    MirrorMode _mirrorMode{ MirrorMode::Horizontal };
};

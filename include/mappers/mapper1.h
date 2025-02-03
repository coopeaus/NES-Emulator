#pragma once
#include "mapper-base.h"

class Mapper1 : public Mapper
{

  public:
    Mapper1( u8 prg_rom_banks, u8 chr_rom_banks );
    auto TranslateCPUAddress( u16 address ) -> u32 override;
    auto TranslatePPUAddress( u16 address ) -> u32 override;
    void HandleCPUWrite( u16 address, u8 data ) override;

    [[nodiscard]] bool SupportsPrgRam() override { return true; }
    [[nodiscard]] bool HasExpansionRom() override { return false; }
    [[nodiscard]] bool HasExpansionRam() override { return false; }

    [[nodiscard]] MirrorMode GetMirrorMode() override;

  private:
    u8 _control_register{ 0x1C };

    // PRG bank selectors.
    u8 _prg_bank_16_lo{ 0 };
    u8 _prg_bank_16_hi;
    u8 _prg_bank_32{ 0 };

    // CHR bank selectors
    u8 _chr_bank_4_lo{ 0 };
    u8 _chr_bank_4_hi{ 0 };
    u8 _chr_bank_8{ 0 };

    // Serial loading mechanism
    u8 _shift_register{ 0x10 };
    u8 _write_count{ 0 };

    // Mirroring
    MirrorMode _mirror_mode{ MirrorMode::Horizontal };
};

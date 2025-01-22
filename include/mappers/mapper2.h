#pragma once
#include "mapper-base.h"

class Mapper2 : public Mapper
{

  public:
    Mapper2( u8 prg_rom_banks, u8 chr_rom_banks, MirrorMode mirror_mode );
    auto TranslateCPUAddress( u16 address ) -> u32 override;
    auto TranslatePPUAddress( u16 address ) -> u32 override;
    void HandleCPUWrite( u16 address, u8 data ) override;

    [[nodiscard]] bool SupportsPrgRam() override { return true; }
    [[nodiscard]] bool HasExpansionRom() override { return false; }
    [[nodiscard]] bool HasExpansionRam() override { return false; }

    [[nodiscard]] MirrorMode GetMirrorMode() override;

  private:
    u8 _prg_bank_16_lo{ 0 };

    // Mirroring mode (fixed)
    MirrorMode _mirror_mode;
};
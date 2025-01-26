#include "ppu.h"

PPU::PPU() = default;

[[nodiscard]]u8 PPU::HandleCpuRead(u16 address)
    {
        return _ppuRegisters[address];
    }

    void PPU::HandleCpuWrite(u16 address, u8 data)
    {
             _ppuRegisters[address] = data;
    }

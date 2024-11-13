#pragma once

#include "cpu.h"
#include <array>

// Forward declarations for components
class PPU;
class APU;
class SaveRAM;
class Cartridge;

class Bus
{
  public:
    // Constructor to initialize the bus with all components (stubbing components here)
    Bus();

    // Memory read/write interface
    [[nodiscard]] u8 Read( uint16_t address ) const;
    void             Write( u16 address, u8 data );

private:
    // Flat memory (64KB for early testing)
    std::array<u8, 65536> _flat_memory;

    // Stubbed out components
    PPU* _ppu = nullptr;
    APU* _apu = nullptr;
    SaveRAM* _save_ram = nullptr;
    Cartridge* _cartridge = nullptr;
};

#pragma once
#include "cpu.h"
#include <array>
#include <cstdint>
#include <memory>

using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;

// forward declarations
class Cartridge;
class PPU;
class APU;

class Bus
{
  public:
    // Will eventually pass other components to the constructor
    // Bus( PPU &ppu, APU &apu, bool use_flat_memory = false );

    // Initialized with flat memory disabled by default. Enabled in json tests only
    Bus( bool use_flat_memory = false );

    // Memory read/write interface
    [[nodiscard]] u8 Read( uint16_t address ) const;
    void             Write( u16 address, u8 data );

    // Load or change the cartridge during runtime
    void LoadCartridge( std::shared_ptr<Cartridge> cartridge );

  private:
    // Shared ownership for dynamic life cycle management, leave off for now
    std::shared_ptr<Cartridge> _cartridge;

    // APU and PPU stubs
    /* PPU &_ppu; */
    /* APU &_apu; */

    // Flat memory for early implementation
    bool                  _use_flat_memory; // For testing purposes
    std::array<u8, 65536> _flat_memory{};   // 64KB memory, for early testing

    // CPU RAM, this can stay in this file
    std::array<u8, 0x0800> _ram{}; // 2KB internal cpu RAM

    // Stubs
    std::array<u8, 0x2000> _ppu_memory{};    // 8KB PPU memory (temp)
    std::array<u8, 0x0020> _apu_io_memory{}; // 32 bytes APU and I/O registers
};

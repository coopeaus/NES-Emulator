// bus.h
#pragma once

#include "cpu.h"
#include <array>
#include <cstdint>

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

    // For now, let's use flat memory. Disabled by default, but but can and should be enabled
    // during testing
    Bus( bool use_flat_memory = false );

    // Memory read/write interface
    [[nodiscard]] u8 Read( uint16_t address ) const;
    void             Write( u16 address, u8 data );

    // Load or change the cartridge during runtime
    void LoadCartridge( std::shared_ptr<Cartridge> cartridge );

  private:
    // Shared ownership for dynamic life cycle management, leave off for now
    /* std::shared_ptr<Cartridge> _cartridge; */

    // Leave these off for now
    /* PPU &_ppu; */
    /* APU &_apu; */

    // Flat memory for early implementation
    bool                  _use_flat_memory; // For testing purposes
    std::array<u8, 65536> _flat_memory{};   // 64KB memory, for early testing

    // CPU RAM, this can stay in this file
    std::array<u8, 0x0800> _ram{}; // 2KB internal cpu RAM

    // Temp memory placeholders. These will eventually be passed of to their parent class Read/Write
    // methods
    std::array<u8, 0x2000> _ppu_memory{};           // 8KB PPU memory (temp)
    std::array<u8, 0x0020> _apu_io_memory{};        // 32 bytes APU and I/O registers
    std::array<u8, 0x1FE0> _expansion_rom_memory{}; // 8160 bytes expansion ROM memory
    std::array<u8, 0x2000> _sram_memory{};          // 8KB save RAM memory
    std::array<u8, 0x8000> _prg_rom_memory{};       // 32KB PRG ROM memory
};
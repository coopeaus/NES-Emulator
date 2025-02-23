#pragma once
#include "cpu.h"
#include "ppu.h"
#include <array>
#include <cstdint>
#include <memory>

using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;
using s16 = int16_t;

class Cartridge;
class CPU;
class PPU;
class APU;

class Bus
{
  public:
    // Initialized with flat memory disabled by default. Enabled in json tests only
    Bus();

    /*
    ################################
    ||         Peripherals        ||
    ################################
    */
    CPU                        cpu;
    PPU                        ppu;
    std::shared_ptr<Cartridge> cartridge;

    /*
    ################################
    ||         Bus Methods        ||
    ################################
    */
    [[nodiscard]] u8 Read( uint16_t address, bool debugMode = false );
    void             Write( u16 address, u8 data );
    void             LoadCartridge( std::shared_ptr<Cartridge> cartridge );

    /*
    ################################
    ||        Debug Methods       ||
    ################################
    */
    [[nodiscard]] bool IsTestMode() const;
    void               DebugReset();
    void               EnableJsonTestMode() { _useFlatMemory = true; }
    void               DisableJsonTestMode() { _useFlatMemory = false; }

  private:
    /*
    ################################
    ||           CPU RAM          ||
    ################################
    */
    std::array<u8, 2048> _ram{}; // 2KB internal cpu RAM

    /*
    ################################
    ||       Debug Variables      ||
    ################################
    */
    bool                  _useFlatMemory{}; // For testing purposes
    std::array<u8, 65536> _flatMemory{};    // 64KB memory, for early testing

    /*
    ################################
    ||       Temporary Stubs      ||
    ################################
    */
    std::array<u8, 32> _apuIoMemory{}; // 32 bytes APU and I/O registers
};

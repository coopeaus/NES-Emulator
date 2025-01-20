#pragma once

#include "mappers/mapper-base.h"
#include <array>
#include <string>
#include <vector>
#include <memory>

using namespace std;

class Cartridge
{
  public:
    Cartridge( const string &file_path );

    // Rule of 5
    Cartridge( const Cartridge & ) = delete;
    Cartridge &operator=( const Cartridge & ) = delete;
    Cartridge( Cartridge && ) = delete;
    Cartridge &operator=( Cartridge && ) = delete;
    ~Cartridge() = default;

    [[nodiscard]] u8 Read( u16 address );
    void             Write( u16 address, u8 data );

  private:
    // Reads
    [[nodiscard]] u8 ReadChrROM( u16 address );       // 0x0000 - 0x1FFF: PPU
    [[nodiscard]] u8 ReadExpansionROM( u16 address ); // 0x4020 - 0x5FFF: CPU
    [[nodiscard]] u8 ReadPrgRAM( u16 address );       // 0x6000 - 0x7FFF: CPU
    [[nodiscard]] u8 ReadPrgROM( u16 address );       // 0x8000 - 0xFFFF: CPU

    // Writes
    void WriteChrRAM( u16 address, u8 data );       // 0x0000 - 0x1FFF: PPU
    void WriteExpansionRAM( u16 address, u8 data ); // 0x4020 - 0x5FFF: CPU
    void WritePrgRAM( u16 address, u8 data );       // 0x6000 - 0x7FFF: CPU
    void WritePrgROM( u16 address, u8 data );       // 0x8000 - 0xFFFF: CPU

    [[nodiscard]] MirrorMode GetMirrorMode();

    // PRG ROM: Program ROM
    vector<u8> _prg_rom;

    /* CHR ROM and RAM
      These store pattern table data

      Games use either CHR ROM or CHR RAM, but never both
      If CHR ROM is not present, CHR RAM is used by default
      CHR ROM is read-only, while CHR RAM is writable.

      In both cases, addresses $0000 - $1FFF are addressed by the PPU,
      and used to store pattern tables:
      $0000 - $0FFF: Pattern Table 0, background tiles
      $1000 - $1FFF: Pattern Table 1, sprite tiles

      If a game uses CHR RAM, it starts empty and the CPU fills in the pattern
      table data dynamically. Having dynamic pattern tables allowed devs to
      create dynamic tiles, though they had to be careful to avoid overwriting
      essential data.
    */

    vector<u8>        _chr_rom;
    array<u8, 0x1FFF> _chr_ram{}; // 8192 bytes (8 KiB)

    // PRG RAM: Program RAM, also known as Save RAM (SRAM) or Work RAM sometimes
    // Its usage is determined by the mapper
    array<u8, 0x1FFF> _prg_ram{}; // 8KiB PRG RAM, also known as Save RAM (SRAM) or Work RAM sometimes

    // Expansion ROM
    // Almost never used, but here it is anyway.
    // Can be both ROM or RAM, determined by the mapper
    array<u8, 0x1FFF> _expansion_memory{};

    // Mapper
    shared_ptr<Mapper> _mapper;

    // Cartridge flags
    u8   _has_battery = 0;
    u8   _four_screen_mode = 0;
    u8   _mirror_mode = 0;
    bool _uses_chr_ram = false;
};

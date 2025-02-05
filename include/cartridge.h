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
    Cartridge( const string &filePath );

    /*
    ################################
    ||          Operators         ||
    ################################
    */
    Cartridge( const Cartridge & ) = delete;
    Cartridge &operator=( const Cartridge & ) = delete;
    Cartridge( Cartridge && ) = delete;
    Cartridge &operator=( Cartridge && ) = delete;
    ~Cartridge() = default;

    /*
    ################################
    ||            Reads           ||
    ################################
    */
    [[nodiscard]] u8 Read( u16 address );
    [[nodiscard]] u8 ReadChrROM( u16 address );        // 0x0000 - 0x1FFF: PPU
    [[nodiscard]] u8 ReadCartridgeVRAM( u16 address ); // 0x2800 - 0x2FFF: PPU (four screen mode)
    [[nodiscard]] u8 ReadExpansionROM( u16 address );  // 0x4020 - 0x5FFF: CPU
    [[nodiscard]] u8 ReadPrgRAM( u16 address );        // 0x6000 - 0x7FFF: CPU
    [[nodiscard]] u8 ReadPrgROM( u16 address );        // 0x8000 - 0xFFFF: CPU

    /*
    ################################
    ||           Writes           ||
    ################################
    */
    void Write( u16 address, u8 data );
    void WriteChrRAM( u16 address, u8 data );        // 0x0000 - 0x1FFF: PPU
    void WriteCartridgeVRAM( u16 address, u8 data ); // 0x2800 - 0x2FFF: PPU (four screen mode)
    void WriteExpansionRAM( u16 address, u8 data );  // 0x4020 - 0x5FFF: CPU
    void WritePrgRAM( u16 address, u8 data );        // 0x6000 - 0x7FFF: CPU
    void WritePrgROM( u16 address, u8 data );        // 0x8000 - 0xFFFF: CPU

    /*
    ################################
    ||      Cartridge Methods     ||
    ################################
    */
    [[nodiscard]] MirrorMode GetMirrorMode();

  private:
    /*
    ################################
    ||      Memory Variables      ||
    ################################
    */

    /* PRG ROM
      Program Read-Only Memory, stores the game's code
      Each cartridge provides 16KiB PRG ROM banks. For simpler cartridges
      The size is fixed, but for others, many banks are provided.
      The iNes header specifies how many PRG ROM banks are provided, so we
      can define _prg_rom as a vector and resize it during ROM initialization
    */
    vector<u8> _prgRom;

    /* CHR ROM and RAM
      Character Read-Only Memory and Character Random Access Memory
      These store pattern table data. There are two pattern tables,
      each 4KiB in size. Pattern tables store 256 8x8 tiles, which the PPU
      uses to render backgrounds and sprites

      Games use either CHR ROM or CHR RAM, but never both
      If CHR ROM is not present, CHR RAM is used by default
      CHR ROM is read-only, while CHR RAM is writable.

      In both cases, addresses $0000 - $1FFF are addressed by the PPU,
      and used to store pattern tables:
      $0000 - $0FFF: Pattern Table 0, background tiles
      $1000 - $1FFF: Pattern Table 1, sprite tiles

      If a game uses CHR RAM, it starts empty and the CPU fills in the pattern
      table data dynamically. Having dynamic pattern tables allowed devs to
      create dynamic tiles.
    */
    vector<u8>        _chrRom;   // Sized based on the number of CHR ROM banks
    array<u8, 0x1FFF> _chrRam{}; // 8192 bytes (8 KiB)

    // PRG RAM: Program RAM, also known as Save RAM (SRAM) or Work RAM sometimes
    // Its usage is determined by the mapper
    array<u8, 0x1FFF> _prgRam{}; // 8KiB PRG RAM, also known as Save RAM (SRAM) or Work RAM sometimes

    // Expansion ROM
    // Almost never used, but here it is anyway.
    // Can be both ROM or RAM, determined by the mapper
    array<u8, 0x1FFF> _expansionMemory{};

    // Cartrdige VRAM
    // The PPU has 2KiB of vram for nametables (background layout information).
    // Some cartridges provided 2Kib extra which allowed for four unique
    // nametables without mirroring. Nametables are documented in the PPU class.
    array<u8, 2048> _cartridgeVram{};

    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
    shared_ptr<Mapper> _mapper;
    u8                 _mapperNumber = 0;
    u8                 _hasBattery = 0;
    bool               _fourScreenMode = false;
    MirrorMode         _mirrorMode = MirrorMode::Vertical;
    bool               _usesChrRam = false;
};

#pragma once

#include "cpu.h"
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

    [[nodiscard]] u8 GetMirrorMode();

    // PRG and CHR data
    vector<u8>      _prg_rom;
    vector<u8>      _chr_rom;
    array<u8, 8192> _chr_ram{}; // 8 KiB CHR RAM, used when CHR ROM is not present

    // Usage will be determined by individual mapper
    array<u8, 8192> _prg_ram{}; // 8KiB PRG RAM, also known as Save RAM (SRAM) or Work RAM sometimes
    array<u8, 8160> _expansion_memory{}; // 8160 bytes expansion memory. Rarely used, but
                                         // sometimes used as RAM, ROM, or both

    // Mapper
    shared_ptr<Mapper> _mapper;

    // Cartridge flags
    u8   _has_battery = 0;
    u8   _four_screen_mode = 0;
    u8   _mirror_mode = 0;
    bool _uses_chr_ram = false;

    enum MirrorMode : u8
    {
        Horizontal,
        Vertical,
        FourScreen
    };
};

#pragma once

#include "global-types.h"
#include "mappers/mapper-base.h"
#include <array>
#include <string>
#include <vector>
#include <memory>
#include "cartridge-header.h"
#include "mappers/mapper1.h"
#include "mappers/mapper2.h"
#include "mappers/mapper3.h"
#include "mappers/mapper4.h"

class Bus;

class Cartridge
{
public:
  Cartridge( Bus *bus );
  iNes2Instance iNes;

  Bus *bus;

  template <class Archive> void save( Archive &ar ) const // NOLINT
  {
    ar( _chrRam, _prgRam, _expansionMemory, romHash );
    int const m = iNes.GetMapper();
    ar( m );
    switch ( m ) {
      case 1: {
        auto m1 = std::static_pointer_cast<Mapper1>( _mapper );
        ar( m1->controlRegister, m1->prgBank16Lo, m1->prgBank16Hi, m1->prgBank32, m1->chrBank4Lo, m1->chrBank4Hi,
            m1->chrBank8, m1->shiftRegister, m1->writeCount, m1->mirroring );
        break;
      }
      case 2: {
        auto m2 = std::static_pointer_cast<Mapper2>( _mapper );
        ar( m2->prgBank16Lo, m2->mirroring );
        break;
      }
      case 3: {
        auto m3 = std::static_pointer_cast<Mapper3>( _mapper );
        ar( m3->chrBank, m3->mirroring );
        break;
      }
      case 4: {
        auto m4 = std::static_pointer_cast<Mapper4>( _mapper );
        ar( m4->nTargetRegister, m4->bPrgBankMode, m4->bChrInversion, m4->pRegister, m4->pChrBank, m4->pPrgBank,
            m4->bIsIrqRequested, m4->bIrqEnabled, m4->nIrqCounter, m4->nIrqReload, m4->mirroring );
        break;
      }
      default:
    }
  }
  template <class Archive> void load( Archive &ar ) // NOLINT
  {
    ar( _chrRam, _prgRam, _expansionMemory, romHash );
    int m = 0;
    ar( m );
    switch ( m ) {
      case 1: {
        _mapper = std::make_shared<Mapper1>( iNes );
        auto m1 = std::static_pointer_cast<Mapper1>( _mapper );
        ar( m1->controlRegister, m1->prgBank16Lo, m1->prgBank16Hi, m1->prgBank32, m1->chrBank4Lo, m1->chrBank4Hi,
            m1->chrBank8, m1->shiftRegister, m1->writeCount, m1->mirroring );
        break;
      }
      case 2: {
        _mapper = std::make_shared<Mapper2>( iNes );
        auto m2 = std::static_pointer_cast<Mapper2>( _mapper );
        ar( m2->prgBank16Lo, m2->mirroring );
        break;
      }
      case 3: {
        _mapper = std::make_shared<Mapper3>( iNes );
        auto m3 = std::static_pointer_cast<Mapper3>( _mapper );
        ar( m3->chrBank, m3->mirroring );
        break;
      }
      case 4: {
        _mapper = std::make_shared<Mapper4>( iNes );
        auto m4 = std::static_pointer_cast<Mapper4>( _mapper );
        ar( m4->nTargetRegister, m4->bPrgBankMode, m4->bChrInversion, m4->pRegister, m4->pChrBank, m4->pPrgBank,
            m4->bIsIrqRequested, m4->bIrqEnabled, m4->nIrqCounter, m4->nIrqReload, m4->mirroring );
        break;
      }
      default:
    }
  }

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
  u8 Read( u16 address );
  u8 ReadChrROM( u16 address );       // 0x0000 - 0x1FFF: PPU
  u8 ReadExpansionROM( u16 address ); // 0x4020 - 0x5FFF: CPU
  u8 ReadPrgRAM( u16 address );       // 0x6000 - 0x7FFF: CPU
  u8 ReadPrgROM( u16 address );       // 0x8000 - 0xFFFF: CPU

  /*
  ################################
  ||           Writes           ||
  ################################
  */
  void Write( u16 address, u8 data );
  void WriteChrRAM( u16 address, u8 data );       // 0x0000 - 0x1FFF: PPU
  void WriteExpansionRAM( u16 address, u8 data ); // 0x4020 - 0x5FFF: CPU
  void WritePrgRAM( u16 address, u8 data );       // 0x6000 - 0x7FFF: CPU
  void WritePrgROM( u16 address, u8 data );       // 0x8000 - 0xFFFF: CPU

  /*
  ################################
  ||      Cartridge Methods     ||
  ################################
  */
  MirrorMode GetMirrorMode();
  void       LoadRom( const std::string &filePath );
  bool       IsRomValid( const std::string &filePath );

  std::shared_ptr<Mapper> GetMapper() const { return _mapper; }
  u8                      GetMapperNum() const { return _mapperNumber; }

  void SaveBatteryRam();
  void LoadBatteryRam();

  /*
  ################################
  ||        Debug Methods       ||
  ################################
  */
  bool DidMapperLoad() const { return didMapperLoad; }
  bool DoesMapperExist() const { return _mapper != nullptr; }
  void SetChrROM( u16 address, u8 data ) { _chrRom.at( address ) = data; }
  /*
  ################################
  ||       Debug Variables      ||
  ################################
  */
  bool didMapperLoad = false;
  void Reset();

  /*
  ################################
  ||      Public Variables      ||
  ################################
  */

  std::string romHash;
  std::string GetRomHash() const { return romHash; }

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
  std::vector<u8> _prgRom = std::vector<u8>( 16384 ); // 16KiB

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
  std::vector<u8>      _chrRom = std::vector<u8>( 8192 ); // 8KiB
  std::array<u8, 8192> _chrRam{};                         // 8192 bytes (8 KiB)

  // PRG RAM: Program RAM, also known as Save RAM (SRAM) or Work RAM sometimes
  // Its usage is determined by the mapper
  std::array<u8, 8192> _prgRam{}; // 8KiB PRG RAM, also known as Save RAM (SRAM) or Work RAM sometimes

  // Expansion ROM
  // Almost never used, but here it is anyway.
  // Can be both ROM or RAM, determined by the mapper
  std::array<u8, 8192> _expansionMemory{};

  // Cartrdige VRAM
  // The PPU has 2KiB of vram for nametables (background layout information).
  // Some cartridges provided 2Kib extra which allowed for four unique
  // nametables without mirroring. Nametables are documented in the PPU class.
  // array<u8, 2048> _cartridgeVram{}; // For simplicity, I've defined all nametables in the PPU class.

  /*
  ################################
  ||      Private Variables     ||
  ################################
  */
  std::shared_ptr<Mapper> _mapper;
  u8                      _mapperNumber = 0;
  std::string             _romPath;
  bool                    _usesChrRam = false;
};

#include "cartridge.h"
#include <array>
#include <cstring>
#include <fmt/base.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "global-types.h"
#include "utils.h"

// Mappers
#include "mappers/mapper-base.h"
#include "mappers/mapper0.h"
#include "mappers/mapper1.h"
#include "mappers/mapper2.h"
#include "mappers/mapper3.h"

Cartridge::Cartridge( Bus *bus ) : bus( bus )
{
}

bool Cartridge::IsRomValid( const std::string &filePath )
{
  std::ifstream romFile( filePath, std::ios::binary );
  if ( !romFile.is_open() ) {
    return false;
  }
  std::array<char, 16> header{};
  if ( !romFile.read( header.data(), header.size() ) ) {
    return false;
  }
  memcpy( iNes.header.value, header.data(), header.size() );
  return iNes.GetIdentification() == "NES\x1A";
}

void Cartridge::LoadRom( const std::string &filePath )
{
  /** @brief Initiates a cartridge and loads a ROM from file
   */
  didMapperLoad = false;
  _romPath = filePath;
  std::ifstream romFile( filePath, std::ios::binary );
  if ( !romFile.is_open() ) {
    throw std::runtime_error( "Failed to open ROM file: " + filePath );
  }

  // Read in the iNES header
  std::array<char, 16> header{};
  if ( !romFile.read( header.data(), header.size() ) ) {
    if ( romFile.eof() ) {
      throw std::runtime_error( "Failed to read ROM header: Unexpected end of file." );
    }
    if ( romFile.fail() ) {
      throw std::runtime_error( "Failed to read ROM header: I/O error." );
    }
    if ( romFile.bad() ) {
      throw std::runtime_error( "Failed to read ROM header: Fatal I/O error." );
    }
  }

  // Load in a unique rom hash
  romHash = utils::GetRomHash( filePath );

  memcpy( iNes.header.value, header.data(), header.size() );

  if ( iNes.GetIdentification() != "NES\x1A" ) {
    throw std::runtime_error( "Invalid ROM file" );
  }

  /*
  ################################
  ||                            ||
  ||    Read PRG and CHR ROMS   ||
  ||                            ||
  ################################
  */

  // Skip 512 bytes if there is trainer data
  bool const hasTrainer = iNes.GetTrainerMode() == 1;
  if ( hasTrainer ) {
    romFile.seekg( 512, std::ios::cur );
  }

  int const prgRomSize = iNes.GetPrgRomSizeBytes();
  int const chrRomSize = iNes.GetChrRomSizeBytes();

  // Set the PRG and CHR vector sizes
  if ( prgRomSize > 0 ) {
    _prgRom.resize( prgRomSize );
  }

  // Sometimes, chr rom isn't provided. Some games use chr ram instead.
  _usesChrRam = chrRomSize == 0;
  if ( _usesChrRam ) {
    // _chrRam is already an 8 KiB array, so do nothing
  } else {
    _chrRom.resize( chrRomSize );
  }

  // Read data into the PRG and CHR ROM vectors
  if ( prgRomSize > 0 ) {

    romFile.read( reinterpret_cast<char *>( _prgRom.data() ), // NOLINT
                  static_cast<std::streamsize>( prgRomSize ) );
  } else {
    fmt::print( "Cartridge:romFile.read:No PRG ROM data found. No read occured.\n" );
  }
  if ( !_usesChrRam ) {

    if ( chrRomSize > 0 ) {
      romFile.read( reinterpret_cast<char *>( _chrRom.data() ), // NOLINT
                    static_cast<std::streamsize>( chrRomSize ) );
    } else {
      fmt::print( "Cartridge:romFile.read:No CHR ROM data found. No read occured.\n" );
    }
  }

  /*
  ################################
  ||                            ||
  ||           Mappers          ||
  ||                            ||
  ################################
  */
  auto const mapperNumber = iNes.GetMapper();
  switch ( mapperNumber ) {
    case 0 : _mapper = std::make_shared<Mapper0>( iNes ); break;
    case 1 : _mapper = std::make_shared<Mapper1>( iNes ); break;
    case 2 : _mapper = std::make_shared<Mapper2>( iNes ); break;
    case 3 : _mapper = std::make_shared<Mapper3>( iNes ); break;
    default: throw std::runtime_error( "Unsupported mapper: " + std::to_string( mapperNumber ) );
  };

  if ( _mapper != nullptr ) {
    didMapperLoad = true;
  }

  romFile.close();
}

/*
################################
||                            ||
||   Read / Write Interface   ||
||                            ||
################################
*/
[[nodiscard]] u8 Cartridge::Read( u16 address )
{
  /** @brief Reads from the cartridge
   * This function is called by the CPU and PPU to read data from the cartridge
   */

  // From the PPU
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    return ReadChrROM( address );
  }

  // From the CPU
  if ( address >= 0x4020 && address <= 0x5FFF ) {
    return ReadExpansionROM( address );
  }
  if ( address >= 0x6000 && address <= 0x7FFF ) {
    return ReadPrgRAM( address );
  }
  if ( address >= 0x8000 && address <= 0xFFFF ) {
    return ReadPrgROM( address );
  }
  return 0xFF;
}

void Cartridge::Write( u16 address, u8 data )
{
  /** @brief Writes to the cartridge
   * This function is called by the CPU and PPU to write data to the cartridge
   */
  // From the PPU
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    WriteChrRAM( address, data );
    return;
  }

  // From the CPU
  if ( address >= 0x4020 && address <= 0x5FFF ) {
    WriteExpansionRAM( address, data );
    return;
  }
  if ( address >= 0x6000 && address <= 0x7FFF ) {
    WritePrgRAM( address, data );
    return;
  }
  if ( address >= 0x8000 && address <= 0xFFFF ) {
    WritePrgROM( address, data );
    return;
  }
}

/*
################################
||                            ||
||       Reads Internal       ||
||                            ||
################################
*/

[[nodiscard]] u8 Cartridge::ReadPrgROM( u16 address )
{
  /** @brief Reads from the PRG ROM, ranges from 0x8000 to 0xFFFF
   * PRG ROM is the program ROM, which contains the game code
   */
  if ( address >= 0x8000 && address <= 0xFFFF ) {
    if ( _mapper == nullptr ) {
      fmt::print( "Cartridge:ReadPrgROM:Mapper is null. Rom file was likely not loaded.\n" );
      return _prgRom.at( address & 0x3FFF );
    }
    u32 const prgOffset = _mapper->MapPrgOffset( address );
    return _prgRom.at( prgOffset );
  }
  return 0xFF;
}

[[nodiscard]] u8 Cartridge::ReadChrROM( u16 address )
{
  /** @brief Reads from the CHR ROM, ranges from 0x0000 to 0x1FFF
   * CHR ROM is the character ROM, which contains the graphics data for the PPU
   */
  if ( address > 0x1FFF )
    return 0xFF;

  if ( _mapper == nullptr ) {
    fmt::print( "Cartridge:ReadChrROM:Mapper is null. Rom file was likely not loaded.\n" );
    return _chrRom.at( address & 0x1FFF );
  }

  u32 const chrOffset = _mapper->MapChrOffset( address );
  if ( _usesChrRam ) {
    return _chrRam.at( chrOffset );
  }
  return _chrRom.at( chrOffset );
}

[[nodiscard]] u8 Cartridge::ReadPrgRAM( u16 address )
{
  /** @brief Reads from the PRG RAM, ranges from 0x6000 to 0x7FFF
   * This is usually used for save data, but any game that uses PRG RAM
   * Not every game uses PRG RAM. In iNes 2.0, presence of PRG RAM is indicated by
   * the 8th bit in the header. However, to keep compatibility with iNes 1.0, whether
   * a game uses PRG RAM or not will be determined by the mapper.
   */
  if ( _mapper == nullptr ) {
    fmt::print( "Cartridge:ReadPrgRAM:Mapper is null. Rom file was likely not loaded.\n" );
    return _prgRam.at( address - 0x6000 );
  }
  if ( address >= 0x6000 && address <= 0x7FFF && _mapper->SupportsPrgRam() ) {
    return _prgRam.at( address - 0x6000 );
  }
  return 0xFF;
}

[[nodiscard]] u8 Cartridge::ReadExpansionROM( u16 address )
{
  /** @brief Reads from the expansion ROM, ranges from 0x4020 to 0x5FFF
   * Expansion ROM is rarely used, but when it is, it's used for additional program data
   */
  if ( _mapper == nullptr ) {
    fmt::print( "Cartridge:ReadExpansionROM:Mapper is null. Rom file was likely not loaded.\n" );
    return _expansionMemory.at( address - 0x4020 );
  }

  if ( address >= 0x4020 && address <= 0x5FFF && _mapper->HasExpansionRom() ) {
    return _expansionMemory.at( address - 0x4020 );
  }
  return 0xFF;
}

/*
################################
||                            ||
||       Write Internal       ||
||                            ||
################################
*/
void Cartridge::WritePrgROM( u16 address, u8 data )
{
  /** @brief Writes to the PRG ROM, ranges from 0x8000 to 0xFFFF
   * PRG ROM is ready-only. However, many mappers use writes to the ROM
   * to trigger bank switching.
   */
  if ( _mapper == nullptr ) {
    fmt::print( "Cartridge:WritePrgROM:Mapper is null. Rom file was likely not loaded.\n" );
    return;
  }

  if ( address >= 0x8000 && address <= 0xFFFF ) {
    _mapper->HandleCPUWrite( address, data );
  }
}

void Cartridge::WriteChrRAM( u16 address, u8 data )
{
  /** @brief Writes to the CHR memory (used for graphics), mapped to the PPU address space
   *
   * CHR RAM resides in the cartridge and is used by the PPU for graphics
   * This function is called by the PPU, not the CPU, to write data to CHR-RAM when the game
   * uses CHR-RAM instead of CHR-ROM
   *
   * The CPU cannot access CHR memory directly. It writes to the PPU registers ($2006 and
   * $2007), and the PPU will call this method.
   *
   * Address Translation:
   * - PPU provdes an address in its memory space ($0000 - $1FFF)
   * - The assigned mapper handles the translation, and writes to the CHR-RAM array
   *
   * Write Logic:
   * - If the cartridge uses CHR-RAM, write work as described above
   * - If the cartridge uses CHR-ROM, writes are ignored
   * - It's either one or the other, never both.
   * - Byte 5 provides the number of CHR-ROM banks. If 0, then that's a signal that a game
   * uses CHR RAM
   */
  if ( _usesChrRam && address >= 0x0000 && address <= 0x1FFF ) {
    if ( _mapper == nullptr ) {
      fmt::print( "Cartridge:WriteChrRAM:Mapper is null. Rom file was likely not loaded.\n" );
      return;
    }
    u16 const translatedAddress = _mapper->MapChrOffset( address );
    _chrRam.at( translatedAddress & 0x1FFF ) = data;
  }
}

void Cartridge::WritePrgRAM( u16 address, u8 data )
{
  /** @brief Writes to the PRG RAM, ranges from 0x6000 to 0x7FFF
   * This is usually used for save data, but any game that uses PRG RAM can use this
   * Not every game uses PRG RAM. In iNes 2.0, presence of PRG RAM is indicated by
   * the 8th bit in the header. However, to keep compatibility with iNes 1.0, whether
   * a game uses PRG RAM or not will be determined by the mapper.
   */
  if ( _mapper == nullptr ) {
    fmt::print( "Cartridge:WritePrgRAM:Mapper is null. Rom file was likely not loaded.\n" );
    return;
  }

  if ( address >= 0x6000 && address <= 0x7FFF && _mapper->SupportsPrgRam() ) {

    _prgRam.at( address - 0x6000 ) = data;
  }
}

void Cartridge::WriteExpansionRAM( u16 address, u8 data )
{
  /** @brief Writes to the expansion ROM, ranges from 0x4020 to 0x5FFF
   * Expansion ROM is rarely used, but when it is, it's used for additional program data
   */
  if ( _mapper == nullptr ) {
    fmt::print( "Cartridge:WriteExpansionRAM:Mapper is null. Rom file was likely not loaded.\n" );
    return;
  }

  if ( address >= 0x4020 && address <= 0x5FFF && _mapper->HasExpansionRam() ) {
    _expansionMemory.at( address - 0x4020 ) = data;
  }
}

/*
################################
||                            ||
||        Other Methods       ||
||                            ||
################################
*/

MirrorMode Cartridge::GetMirrorMode()
{
  /** @brief Returns the mirror mode of the cartridge
   * The mirror mode determines how the PPU should handle nametable mirroring.
   */
  if ( _mapper == nullptr ) {
    return MirrorMode::Vertical;
  }

  return _mapper->GetMirrorMode();
}

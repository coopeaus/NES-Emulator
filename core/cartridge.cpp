#include "cartridge.h"
#include <array>
#include <cstddef>
#include <cstring>
#include <fmt/base.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// Mappers
#include "global-types.h"
#include "mappers/mapper-base.h"
#include "mappers/mapper0.h"
#include "mappers/mapper1.h"

Cartridge::Cartridge( Bus *bus ) : bus( bus )
{
}

void Cartridge::LoadRom( const std::string &filePath )
{
  /** @brief Initiates a cartridge and loads a ROM from file
   */
  didMapperLoad = false;

  std::ifstream romFile( filePath, std::ios::binary );
  if ( !romFile.is_open() ) {
    throw std::runtime_error( "Failed to open ROM file: " + filePath );
  }

  // Bail if the ROM is larger than 5 MiB
  constexpr size_t maxRomSize = static_cast<const size_t>( 5 * 1024 * 1024 );
  romFile.seekg( 0, std::ios::end );
  size_t const fileSize = static_cast<size_t>( romFile.tellg() );
  if ( fileSize > maxRomSize ) {
    throw std::runtime_error( "ROM file too large" );
  }
  romFile.seekg( 0, std::ios::beg );

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

  memcpy( iNes.header.value, header.data(), header.size() );

  if ( iNes.GetIdentification() != "NES\x1A" ) {
    throw std::runtime_error( "Invalid ROM file" );
  }

  /*
  ################################
  ||                            ||
  ||         Header Info        ||
  ||                            ||
  ################################
  */

  // Mirror mode
  // Provided by the 0th bit of byte 6.
  // u8 const mirrorMode = header[6] & 0b00000001;
  u8 const mirrorMode = iNes.GetMirroring();
  ( mirrorMode == 0 ) ? _mirrorMode = MirrorMode::Horizontal : _mirrorMode = MirrorMode::Vertical;

  // Four screen mode
  _fourScreenMode = iNes.GetFourScreenMode();
  if ( _fourScreenMode ) {
    _mirrorMode = MirrorMode::FourScreen;
  }

  _hasBattery = iNes.GetBatteryMode();

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
  _usesChrRam = chrRomSize == 0;
  if ( prgRomSize > 0 ) {
    _prgRom.resize( prgRomSize );
  }

  // Sometimes, chr rom isn't provided. Some games use chr ram instead.
  if ( !_usesChrRam ) {
    if ( chrRomSize > 0 ) {
      _chrRom.resize( chrRomSize );
    }
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
    u32 const translatedAddress = _mapper->TranslateCPUAddress( address );
    return _prgRom.at( translatedAddress );
  }
  return 0xFF;
}

[[nodiscard]] u8 Cartridge::ReadChrROM( u16 address )
{
  /** @brief Reads from the CHR ROM, ranges from 0x0000 to 0x1FFF
   * CHR ROM is the character ROM, which contains the graphics data for the PPU
   */
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    if ( _mapper == nullptr ) {
      fmt::print( "Cartridge:ReadChrROM:Mapper is null. Rom file was likely not loaded.\n" );
      return _chrRom.at( address & 0x1FFF );
    }
    u32 const translatedAddress = _mapper->TranslatePPUAddress( address );
    return _chrRom.at( translatedAddress );
  }
  return 0xFF;
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
    u16 const translatedAddress = _mapper->TranslatePPUAddress( address );
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
   *
   * Mapper 0:
   *   - Mirroring mode is statically defined in the iNES header (bit 0 of byte 6).
   *   - It cannot change dynamically and is "soldered" for each specific game.
   *
   * Four-Screen Mode:
   *   - Indicated by bit 3 of byte 6 in the iNES header.
   *   - Overrides any mirroring mode (horizontal or vertical) defined in bit 0.
   *   - This mode provides unique nametables for all four screens using extra cartridge VRAM.
   *
   * Other mappers:
   *   - Mirroring mode is controlled dynamically via mapper logic.
   *   - The specific mirroring configuration depends on the mapper implementation.
   */

  // Mapper 0 or Four-Screen Mode: Static mirroring, determined by iNES header
  if ( _mapperNumber == 0 || _fourScreenMode ) {
    return _mirrorMode;
  }

  // Other mappers: Dynamic mirroring, determined by mapper logic
  return _mapper->GetMirrorMode();
}

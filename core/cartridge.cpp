#include "cartridge.h"
#include <array>
#include <cstddef>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include <string>

// Mappers
#include "mappers/mapper-base.h"
#include "mappers/mapper0.h"
#include "mappers/mapper1.h"

Cartridge::Cartridge( const std::string &filePath )
{
    /** @brief Initiates a cartridge and loads a ROM from file
     */
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

    // First four bytes, should be "NES\x1A", if not, let's bail
    if ( header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A ) {
        throw std::runtime_error( "Invalid ROM file" );
    }

    /*
    ################################
    ||                            ||
    ||         Header Info        ||
    ||                            ||
    ################################
    */
    // Flag 6 and 7, provide various info
    u8 const flags6 = header[6];
    u8 const flags7 = header[7];

    // Mirror mode
    // Provided by the 0th bit of byte 6.
    u8 const mirrorMode = header[6] & 0b00000001;
    ( mirrorMode == 0 ) ? _mirrorMode = MirrorMode::Horizontal : _mirrorMode = MirrorMode::Vertical;

    // Four screen mode
    // Provided by the 3rd bit of byte 6.
    // If provided, it overrides info in byte 0
    _fourScreenMode = ( flags6 & 0b00001000 ) != 0;
    if ( _fourScreenMode ) {
        _mirrorMode = MirrorMode::FourScreen;
    }

    // PRG and CHR banks, derived from bytes 4 and 5
    u8 const     prgRomBanks = header[4];
    u8 const     chrRomBanks = header[5];
    size_t const prgRomSize = static_cast<size_t>( header[4] * 16 * 1024 );
    size_t const chrRomSize = static_cast<size_t>( header[5] * 8 * 1024 );

    // Mapper number
    // Derived from the upper 4 bits of byte 7 and the lower 4 bits of byte 6
    u8 const mapperNumber = ( flags7 & 0b11110000 ) | ( flags6 >> 4 );

    // Does it have a save battery?
    // Derived from the 1st bit of byte 6
    _hasBattery = ( flags6 & 0b00000010 );

    // Does it have trainer data?
    // Derived from the 2nd bit of byte 6
    bool const hasTrainer = ( flags6 & 0b00000100 ) != 0;

    /*
    ################################
    ||                            ||
    ||    Read PRG and CHR ROMS   ||
    ||                            ||
    ################################
    */

    // Skip 512 bytes if there is trainer data
    if ( hasTrainer ) {
        romFile.seekg( 512, std::ios::cur );
    }

    // Set the PRG and CHR vector sizes
    _usesChrRam = chrRomSize == 0;
    _prgRom.resize( prgRomSize );
    // Sometimes, chr rom isn't provided. Some games use chr ram instead.
    if ( !_usesChrRam ) {
        _chrRom.resize( chrRomSize );
    }

    // Read data into the PRG and CHR ROM vectors
    romFile.read( reinterpret_cast<char *>( _prgRom.data() ), // NOLINT
                  static_cast<std::streamsize>( prgRomSize ) );
    if ( !_usesChrRam ) {

        romFile.read( reinterpret_cast<char *>( _chrRom.data() ), // NOLINT
                      static_cast<std::streamsize>( chrRomSize ) );
    }

    /*
    ################################
    ||                            ||
    ||           Mappers          ||
    ||                            ||
    ################################
    */
    switch ( mapperNumber ) {
        case 0:
            _mapper = std::make_shared<Mapper0>( prgRomBanks, chrRomBanks );
            break;
        case 1:
            _mapper = std::make_shared<Mapper1>( prgRomBanks, chrRomBanks );
            break;
        default:
            throw std::runtime_error( "Unsupported mapper: " + std::to_string( mapperNumber ) );
    };

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
    if ( address >= 2800 && address <= 0x2FFF ) {
        return ReadCartridgeVRAM( address );
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
    if ( address >= 2800 && address <= 0x2FFF ) {
        WriteCartridgeVRAM( address, data );
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
        u32 const translatedAddress = _mapper->TranslateCPUAddress( address );
        return _prgRom.at(translatedAddress & 0xFFFF);
    }
    return 0xFF;
}

[[nodiscard]] u8 Cartridge::ReadChrROM( u16 address )
{
    /** @brief Reads from the CHR ROM, ranges from 0x0000 to 0x1FFF
     * CHR ROM is the character ROM, which contains the graphics data for the PPU
     */
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        u32 const translatedAddress = _mapper->TranslatePPUAddress( address );
        return _chrRom.at(translatedAddress & 0x1FFF);
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
    if ( address >= 0x6000 && address <= 0x7FFF && _mapper->SupportsPrgRam() ) {
        return _prgRam.at(address - 0x6000);
    }
    return 0xFF;
}

[[nodiscard]] u8 Cartridge::ReadExpansionROM( u16 address )
{
    /** @brief Reads from the expansion ROM, ranges from 0x4020 to 0x5FFF
     * Expansion ROM is rarely used, but when it is, it's used for additional program data
     */
    if ( address >= 0x4020 && address <= 0x5FFF && _mapper->HasExpansionRom() ) {
        return _expansionMemory.at(address - 0x4020);
    }
    return 0xFF;
}

[[nodiscard]] u8 Cartridge::ReadCartridgeVRAM( u16 address )
{
    /** @brief Reads from the cartridge VRAM, ranges from 0x2800 to 0x2FFF
     * This is used by the PPU in four-screen mode
     */
    if ( address >= 0x2800 && address <= 0x2FFF ) {
        return _cartridgeVram.at(address & 0x07FF); // Mask to 2Kib
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
        u16 const translatedAddress = _mapper->TranslatePPUAddress( address );
        _chrRam.at(translatedAddress & 0x1FFF) = data;
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
    if ( address >= 0x6000 && address <= 0x7FFF && _mapper->SupportsPrgRam() ) {

        _prgRam.at(address - 0x6000) = data;
    }
}

void Cartridge::WriteExpansionRAM( u16 address, u8 data )
{
    /** @brief Writes to the expansion ROM, ranges from 0x4020 to 0x5FFF
     * Expansion ROM is rarely used, but when it is, it's used for additional program data
     */
    if ( address >= 0x4020 && address <= 0x5FFF && _mapper->HasExpansionRam() ) {
        _expansionMemory.at(address - 0x4020) = data;
    }
}

void Cartridge::WriteCartridgeVRAM( u16 address, u8 data )
{
    /** @brief Writes to the cartridge VRAM, ranges from 0x2800 to 0x2FFF
     * This is used by the PPU in four-screen mode
     */
    if ( address >= 0x2800 && address <= 0x2FFF ) {
        _cartridgeVram.at(address & 0x07FF) = data;
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

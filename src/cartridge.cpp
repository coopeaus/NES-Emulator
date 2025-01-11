#include "cartridge.h"
#include "cpu.h"
#include <array>
#include <cstddef>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include <string>

// Mappers
#include "mappers/mapper0.h"

Cartridge::Cartridge( const std::string &file_path )
{
    /** @brief Initiates a cartridge and loads a ROM from file
     */
    std::ifstream rom_file( file_path, std::ios::binary );
    if ( !rom_file.is_open() )
    {
        throw std::runtime_error( "Failed to open ROM file: " + file_path );
    }

    // Bail if the ROM is larger than 5 MiB
    constexpr size_t max_rom_size = static_cast<const size_t>( 5 * 1024 * 1024 );
    rom_file.seekg( 0, std::ios::end );
    size_t const file_size = static_cast<size_t>( rom_file.tellg() );
    if ( file_size > max_rom_size )
    {
        throw std::runtime_error( "ROM file too large" );
    }
    rom_file.seekg( 0, std::ios::beg );

    // Read in the iNES header
    std::array<char, 16> header{};
    if ( !rom_file.read( header.data(), header.size() ) )
    {
        if ( rom_file.eof() )
        {
            throw std::runtime_error( "Failed to read ROM header: Unexpected end of file." );
        }
        if ( rom_file.fail() )
        {
            throw std::runtime_error( "Failed to read ROM header: I/O error." );
        }
        if ( rom_file.bad() )
        {
            throw std::runtime_error( "Failed to read ROM header: Fatal I/O error." );
        }
    }

    // First four bytes, should be "NES\x1A", if not, let's bail
    if ( header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A )
    {
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
    _mirror_mode = header[6] & 0b00000001;

    // Four screen mode
    // Provided by the 3rd bit of byte 6.
    _four_screen_mode = ( flags6 & 0b00001000 );

    // PRG banks
    // The number of 16 KiB PRG banks available. Stored in byte 4
    size_t const prg_rom_size = static_cast<size_t>( header[4] * 16 * 1024 );

    // CHR banks, derived from byte 5
    size_t const chr_rom_size = static_cast<size_t>( header[5] * 8 * 1024 );

    // Mapper number
    // Derived from the upper 4 bits of byte 7 and the lower 4 bits of byte 6
    u8 const mapper_number = ( flags7 & 0b11110000 ) | ( flags6 >> 4 );

    // Does it have a save battery?
    // Derived from the 1st bit of byte 6
    _has_battery = ( flags6 & 0b00000010 );

    // Does it have trainer data?
    // Derived from the 2nd bit of byte 6
    bool const has_trainer = ( flags6 & 0b00000100 ) != 0;

    /*
    ################################
    ||                            ||
    ||    Read PRG and CHR ROMS   ||
    ||                            ||
    ################################
    */

    // Skip 512 bytes if there is trainer data
    if ( has_trainer )
    {
        rom_file.seekg( 512, std::ios::cur );
    }

    // Set the PRG and CHR vector sizes
    _uses_chr_ram = chr_rom_size == 0;
    _prg_rom.resize( prg_rom_size );
    // Sometimes, chr rom isn't provided. Some games use chr ram instead.
    if ( !_uses_chr_ram )
    {
        _chr_rom.resize( chr_rom_size );
    }

    // Read data into the PRG and CHR ROM vectors
    rom_file.read( reinterpret_cast<char *>( _prg_rom.data() ), // NOLINT
                   static_cast<std::streamsize>( prg_rom_size ) );
    if ( !_uses_chr_ram )
    {

        rom_file.read( reinterpret_cast<char *>( _chr_rom.data() ), // NOLINT
                       static_cast<std::streamsize>( chr_rom_size ) );
    }

    /*
    ################################
    ||                            ||
    ||           Mappers          ||
    ||                            ||
    ################################
    */
    switch ( mapper_number )
    {
        case 0:
            _mapper = std::make_shared<Mapper0>( prg_rom_size, chr_rom_size );
            break;
        default:
            throw std::runtime_error( "Unsupported mapper: " + std::to_string( mapper_number ) );
    };

    rom_file.close();
}

/*

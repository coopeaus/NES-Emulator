#pragma once

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <unordered_set>
#include <sstream>
#include <regex>
#include <vector>
#include <fstream>
#include <array>

using u8 = std::uint8_t;
using u16 = std::uint16_t;

using namespace std;

namespace utils
{

using MatchResult = std::vector<std::string>;
using MatchResults = std::vector<MatchResult>;
MatchResult  parseLogLine( const std::string &line, const std::regex &pattern, std::size_t expectedMatches );
MatchResults parseLog( const std::string &filename, const std::regex &pattern, std::size_t expectedMatches );

/*
################################
||                            ||
||       String Related       ||
||                            ||
################################
* Various string related utilities
*/
inline std::string toHex( u16 num, u8 width = 4 )
{
    /*
     * @brief Convert a 16-bit unsigned integer to a hexadecimal string
     */

    std::string hexStr( width, '0' );
    for ( int i = width - 1; i >= 0; --i, num >>= 4 ) {
        hexStr[i] = "0123456789ABCDEF"[num & 0xF];
    }
    return hexStr;
}

/*
################################
||                            ||
||  Address String Validator  ||
||                            ||
################################
* Validates address mode strings in the lookup table (cpu.cpp)
*/
inline const std::unordered_set<std::string> &getAddrModeSet()
{
    static const std::unordered_set<std::string> addrModeSet = {
        "IMP", "IMM", "ZPG", "ZPGX", "ZPGY", "ABS", "ABSX", "ABSY", "IND", "INDX", "INDY", "REL" };
    return addrModeSet;
}

inline bool isValidAddrModeStr( const std::string &addrMode )
{
    const auto &addrModeSet = getAddrModeSet();
    return addrModeSet.find( addrMode ) != addrModeSet.end();
}

inline std::string getAvailableAddrModes()
{
    const auto        &addrModeSet = getAddrModeSet();
    std::ostringstream oss;
    for ( const auto &addrMode : addrModeSet ) {
        oss << addrMode << ", ";
    }
    std::string result = oss.str();
    if ( !result.empty() ) {
        result.pop_back();
        result.pop_back();
    }
    return result;
}

/*
################################
||                            ||
||    Opcode Name Validator   ||
||                            ||
################################
* Validates names defined in the opcode lookup table (cpu.cpp)
*/
inline const std::unordered_set<std::string> &getOpcodeNameSet()
{
    static const std::unordered_set<std::string> nameSet = {
        "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS", "CLC",
        "CLD", "CLI", "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP",
        "JSR", "LDA", "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL", "ROR", "RTI",
        "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY", "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",
        // Illegal
        "*ALR", "*ANC", "*ARR", "*DCP", "*ISC", "*JAM", "*LAX", "*LXA", "*NOP", "*RLA", "*RRA", "*SAX",
        "*SBC", "*SBX", "*SLO", "*SRE" };
    return nameSet;
}

inline bool isValidOpcodeName( const std::string &name )
{
    const auto &nameSet = getOpcodeNameSet();
    return nameSet.find( name ) != nameSet.end();
}

inline std::string getAvailableOpcodeNames()
{
    const auto        &nameSet = getOpcodeNameSet();
    std::ostringstream oss;
    for ( const auto &name : nameSet ) {
        oss << name << ", ";
    }
    std::string result = oss.str();
    // Remove trailing comma and space
    if ( !result.empty() ) {
        result.pop_back();
        result.pop_back();
    }
    return result;
}

/*
################################
||                            ||
||        Palette Read        ||
||                            ||
################################
*/

inline array<uint32_t, 64> readPalette( const string &filename )
{
    array<uint32_t, 64> nesPalette{};

    std::ifstream file( filename, std::ios::binary );
    if ( !file ) {
        std::cerr << "utils::readPalette: Failed to open palette file: " << filename << '\n';
        throw std::runtime_error( "Failed to open palette file" );
    }

    file.seekg( 0, std::ios::end );
    streamsize const fileSize = file.tellg();
    if ( fileSize != 192 ) {
        std::cerr << "utils::readPalette: Invalid palette file size: " << fileSize << '\n';
        throw std::runtime_error( "Invalid palette file size" );
    }

    file.seekg( 0, std::ios::beg );

    char buffer[192]; // NOLINT
    if ( !file.read( buffer, 192 ) ) {
        std::cerr << "utils::readPalette: Failed to read palette file: " << filename << '\n';
        throw std::runtime_error( "Failed to read palette file" );
    }

    // Convert to 32-bit RGBA (SDL_PIXELFORMAT_RGBA32)
    for ( int i = 0; i < 64; ++i ) {
        uint8_t const red = buffer[( i * 3 ) + 0];
        uint8_t const green = buffer[( i * 3 ) + 1];
        uint8_t const blue = buffer[( i * 3 ) + 2];
        uint8_t const alpha = 0xFF;
        nesPalette[i] = ( alpha << 24 ) | ( blue << 16 ) | ( green << 8 ) | red;
    }

    return nesPalette;
}

} // namespace utils

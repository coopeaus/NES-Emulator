#pragma once

#include <cstddef>
#include <string>
#include <cstdint>
#include <unordered_set>
#include <sstream>
#include <regex>
#include <vector>

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

    std::string hex_str( width, '0' );
    for ( int i = width - 1; i >= 0; --i, num >>= 4 )
    {
        hex_str[i] = "0123456789ABCDEF"[num & 0xF];
    }
    return hex_str;
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
    static const std::unordered_set<std::string> addr_mode_set = {
        "IMP", "IMM", "ZPG", "ZPGX", "ZPGY", "ABS", "ABSX", "ABSY", "IND", "INDX", "INDY", "REL" };
    return addr_mode_set;
}

inline bool isValidAddrModeStr( const std::string &addr_mode )
{
    const auto &addr_mode_set = getAddrModeSet();
    return addr_mode_set.find( addr_mode ) != addr_mode_set.end();
}

inline std::string getAvailableAddrModes()
{
    const auto        &addr_mode_set = getAddrModeSet();
    std::ostringstream oss;
    for ( const auto &addr_mode : addr_mode_set )
    {
        oss << addr_mode << ", ";
    }
    std::string result = oss.str();
    if ( !result.empty() )
    {
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
    static const std::unordered_set<std::string> name_set = {
        "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS", "CLC",
        "CLD", "CLI", "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP",
        "JSR", "LDA", "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL", "ROR", "RTI",
        "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY", "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",
        // Illegal
        "*ALR", "*ANC", "*ARR", "*DCP", "*ISC", "*JAM", "*LAX", "*LXA", "*NOP", "*RLA", "*RRA", "*SAX",
        "*SBC", "*SBX", "*SLO", "*SRE" };
    return name_set;
}

inline bool isValidOpcodeName( const std::string &name )
{
    const auto &name_set = getOpcodeNameSet();
    return name_set.find( name ) != name_set.end();
}

inline std::string getAvailableOpcodeNames()
{
    const auto        &name_set = getOpcodeNameSet();
    std::ostringstream oss;
    for ( const auto &name : name_set )
    {
        oss << name << ", ";
    }
    std::string result = oss.str();
    // Remove trailing comma and space
    if ( !result.empty() )
    {
        result.pop_back();
        result.pop_back();
    }
    return result;
}

} // namespace utils

#pragma once

#include <cstddef>
#include <string>
#include <regex>
#include <vector>
#include "global-types.h"
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace utils
{

using MatchResult = std::vector<std::string>;
using MatchResults = std::vector<MatchResult>;
MatchResult  parseLogLine( const std::string &line, const std::regex &pattern, std::size_t expectedMatches );
MatchResults parseLog( const std::string &filename, const std::regex &pattern, std::size_t expectedMatches );

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

inline bool between( int value, int min, int max )
{
  /*
   * @brief Check if a value is between min and max
   */
  return ( value >= min && value <= max );
}

inline std::string GetRomHash( const std::string &path ) // NOLINT
{
  /**
   * @brief   FNV-la computes a simple, non-cryptographic fingerprint of a file.
   * A hash can be used to identify a file, which is used for save / load states.
   * 2^64 possible values is enough to avoid collisions for every possible ROM.
   */
  constexpr u64 fnvOffsetBasis = 0xcbf29ce484222325ULL;
  constexpr u64 fnvPrime = 0x00000100000001B3ULL;

  std::ifstream in( path, std::ios::binary );
  if ( !in ) {
    return {};
  }

  uint64_t hash = fnvOffsetBasis;
  char     buf[4096];
  while ( in.read( buf, sizeof( buf ) ) || in.gcount() ) {
    std::streamsize n = in.gcount();
    for ( std::streamsize i = 0; i < n; ++i ) {
      hash ^= static_cast<u8>( buf[i] );
      hash *= fnvPrime;
    }
  }

  std::ostringstream oss;
  oss << std::hex << std::setw( 16 ) << std::setfill( '0' ) << hash;
  return oss.str();
}

} // namespace utils

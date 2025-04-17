#pragma once

#include <cstddef>
#include <string>
#include <regex>
#include <vector>
#include "global-types.h"

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

} // namespace utils

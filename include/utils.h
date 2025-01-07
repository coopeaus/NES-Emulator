#pragma once

#include "cpu.h"
#include <string>

namespace utils
{

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

} // namespace utils

#pragma once

#include "cpu.h"
#include <array>
class Bus

{
  public:
    Bus();

    // Memory read/write interface
    [[nodiscard]] u8 Read( uint16_t address ) const;
    void             Write( u16 address, u8 data );

  private:
    std::array<u8, 65536> _flat_memory{}; // 64KB memory, for early testing
};

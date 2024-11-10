// cpu.h
#pragma once
#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;

// Forward declaration, will be used for reads and writes
class Bus;

class CPU
{
  public:
    explicit CPU( Bus *bus ); // Must pass in pointer to a bus class on initialization

    // Read/write
    [[nodiscard]] auto Read( u16 address ) const -> u8;
    void               Write( u16 address, u8 data );

    // Getters

    // Setters

    // Helper Methods

  private:
    Bus *_bus;

    // Registers

    // Opcode Table

    // Addressing Modes

    // Instruction Helpers
};

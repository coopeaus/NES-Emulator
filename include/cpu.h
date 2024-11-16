#pragma once

#include <cstdint>

// Aliases for integer types
using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;

// enums

// enum for the status registers set to 0 by default
enum Status {
  Carry = 0b0,
  Zero = 0b0,
  Interrupt_Disable = 0b0,
  Decimal = 0b0,
  Overflow = 0b0,
  Negative = 0b0,
  B_Flag = 0b0
}

// Forward declaration for reads and writes
class Bus;

class CPU
{
  public:
    explicit CPU( Bus *bus ); // Must pass a pointer to a Bus class on initialization

    // Read/write methods
    [[nodiscard]] auto Read( u16 address ) const -> u8;
    void               Write( u16 address, u8 data );

    // Getters for registers
    [[nodiscard]] u8  GetAccumulator() const;
    [[nodiscard]] u8  GetXRegister() const;
    [[nodiscard]] u8  GetYRegister() const;
    [[nodiscard]] u8  GetStatusRegister() const;
    [[nodiscard]] u8  GetStackPointer() const;
    [[nodiscard]] u16 GetProgramCounter() const;
    [[nodiscard]] u64 GetCycles() const;

    // Setters for registers
    void SetAccumulator( u8 value );
    void SetXRegister( u8 value );
    void SetYRegister( u8 value );
    void SetStatusRegister( u8 value );
    void SetStackPointer( u8 value );
    void SetProgramCounter( u16 value );
    void SetCycles( u64 value );

  private:
    Bus *_bus; // Pointer to the Bus class

    // Registers
    u16 _pc = 0x0000; // Program counter (PC)
    u8  _a = 0x00;    // Accumulator register (A)
    u8  _x = 0x00;    // X register
    u8  _y = 0x00;    // Y register
    u8  _s = 0xFD;    // Stack pointer (SP)
    u8  _p = 0x00;    // Status register (P)
    u64 _cycles = 0;  // Number of cycles

    // private methods 

    void setFlag(Status flag) {
      flag = 0b1;
    }

    void clearFlag(Status flag) {
      flag = 0b0;
    }

    void isFlagSet(Status flag) {
      return flag == 1? 1: 0;
    }
};

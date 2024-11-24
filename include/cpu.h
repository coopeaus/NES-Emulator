#pragma once

#include <cstdint>

// Aliases for integer types
using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;

// Enum for Status Register

enum Status : u8
  {
    Carry = 1 << 0, // 0b 00000001
    Zero = 1 << 1,  // 0b 00000010
    Interrupt_Disable = 1 << 2,
    Decimal = 1 << 3,
    Overflow = 1 << 4,
    Negative = 1 << 5,
    B_Flag = 1 << 6
  };

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

    //Flag methods

    void SetFlag( u8 flag );
    void ClearFlag( u8 flag );

  //Clear for Flags
    void CLC() const;
    void CLI() const;
    void CLD() const;
    void CLV() const;

  //Setters for Flags
    void SEC();
    void SED();
    void SEI();
    [[nodiscard]] u8   IsFlagSet( u8 flag ) const;

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
};

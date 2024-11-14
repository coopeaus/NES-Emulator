#pragma once

#include <cstdint>

// Aliases for integer types
using u8 = uint8_t;
using u16 = uint16_t;

// Forward declaration for reads and writes
class Bus;

class CPU
{
  public:
    explicit CPU(Bus* bus); // Must pass a pointer to a Bus class on initialization

    // Read/write methods
    [[nodiscard]] auto read(u16 address) const -> u8;
    void write(u16 address, u8 data);

    // Getters for registers
    [[nodiscard]] u8 getAccumulator() const;
    [[nodiscard]] u8 getXRegister() const;
    [[nodiscard]] u8 getYRegister() const;
    [[nodiscard]] u8 getStatusRegister() const;
    [[nodiscard]] u8 getStackPointer() const;
    [[nodiscard]] u16 getProgramCounter() const;

    // Setters for registers
    void setAccumulator(u8 value);
    void setXRegister(u8 value);
    void setYRegister(u8 value);
    void setStatusRegister(u8 value);
    void setStackPointer(u8 value);
    void setProgramCounter(u16 value);

  private:
    Bus* _bus; // Pointer to the Bus class

    // Registers
    u16 _pc = 0x0000; // Program counter (PC)
    u8 _a = 0x00;     // Accumulator register (A)
    u8 _x = 0x00;     // X register
    u8 _y = 0x00;     // Y register
    u8 _s = 0xFF;     // Stack pointer (SP)
    u8 _p = 0x00;     // Status register (P)
};
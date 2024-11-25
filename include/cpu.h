#pragma once

#include <array>
#include <cstdint>
#include <string>

// Aliases for integer types
using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;

// Forward declaration for reads and writes
class Bus;

class CPU
{
  public:
    explicit CPU( Bus *bus ); // Must pass a pointer to a Bus class on initialization

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
    friend class CPUTestFixture; // Sometimes used for testing private methods

    Bus *_bus; // Pointer to the Bus class

    // Registers
    u16 _pc = 0x0000; // Program counter (PC)
    u8  _a = 0x00;    // Accumulator register (A)
    u8  _x = 0x00;    // X register
    u8  _y = 0x00;    // Y register
    u8  _s = 0xFD;    // Stack pointer (SP)
    u8  _p =
        0x00 | Unused; // Status register (P), per the specs, the unused flag should always be set
    u64 _cycles = 0;   // Number of cycles

    // Instruction data
    struct InstructionData
    {
        std::string name;                        // Instruction mnemonic (e.g. LDA, STA)
        void ( CPU::*instructionMethod )( u16 ); // Pointer to the instruction helper method
        u16 ( CPU::*addressingModeMethod )();    // Pointer to the address mode helper method
        u8 cycles;                               // Number of cycles the instruction takes
        // Some instructions take an extra cycle if a page boundary is crossed. However, in some
        // cases the extra cycle is not taken if the operation is a read. This will be set
        // selectively for a handful of opcodes, but otherwise will be set to true by default
        bool pageCrossPenalty = true;
    };

    bool _currentPageCrossPenalty = true;

    // Opcode table
    std::array<InstructionData, 256> _opcodeTable;

    /*
    ################################################################
    ||                                                            ||
    ||                        CPU Methods                         ||
    ||                                                            ||
    ################################################################
    */
    void Reset();

    // Fetch/decode/execute cycle
    [[nodiscard]] u8 Fetch();
    void             Tick();

    // Read/write methods
    [[nodiscard]] auto Read( u16 address ) const -> u8;
    void               Write( u16 address, u8 data ) const;

    /*
    ################################################################
    ||                                                            ||
    ||                    Instruction Helpers                     ||
    ||                                                            ||
    ################################################################
    */

    // Enum for Status Register
    enum Status : u8
    {
        Carry = 1 << 0,            // 0b00000001
        Zero = 1 << 1,             // 0b00000010
        InterruptDisable = 1 << 2, // 0b00000100
        Decimal = 1 << 3,          // 0b00001000
        Break = 1 << 4,            // 0b00010000
        Unused = 1 << 5,           // 0b00100000
        Overflow = 1 << 6,         // 0b01000000
        Negative = 1 << 7,         // 0b10000000
    };

    // Flag methods
    void               SetFlags( u8 flag );
    void               ClearFlags( u8 flag );
    [[nodiscard]] auto IsFlagSet( u8 flag ) const -> bool;
    void               SetZeroAndNegativeFlags( u8 value );

    // LDA, LDX, and LDY helper
    void LoadRegister( u16 address, u8 &reg );
    void StoreRegister( u16 address, u8 reg ) const;

    /*
    ################################################################
    ||                                                            ||
    ||                      Addressing Modes                      ||
    ||                                                            ||
    ################################################################
    */
    auto IMP() -> u16; // Implied //NOLINT

    auto IMM() -> u16;  // Immediate
    auto ZPG() -> u16;  // Zero Page
    auto ZPGX() -> u16; // Zero Page X
    auto ZPGY() -> u16; // Zero Page Y
    auto ABS() -> u16;  // Absolute
    auto ABSX() -> u16; // Absolute X
    auto ABSY() -> u16; // Absolute Y
    auto IND() -> u16;  // Indirect
    auto INDX() -> u16; // Indirect X
    auto INDY() -> u16; // Indirect Y
    auto REL() -> u16;  // Relative

    /*
    ################################################################
    ||                                                            ||
    ||                        Instructions                        ||
    ||                                                            ||
    ################################################################
      */
    void LDA( u16 address );
    void LDX( u16 address );
    void LDY( u16 address );
    void STA( u16 address );
    void STX( u16 address );
    void STY( u16 address );

    // Clear for Flags
    void CLC( u16 address );
    void CLI( u16 address );
    void CLD( u16 address );
    void CLV( u16 address );

    // Setters for Flags
    void SEC( u16 address );
    void SED( u16 address );
    void SEI( u16 address );
    enum Status : u8
    {
        Carry = 1 << 0,            // 0b00000001
        Zero = 1 << 1,             // 0b00000010
        InterruptDisable = 1 << 2, // 0b00000100
        Decimal = 1 << 3,          // 0b00001000
        Break = 1 << 4,            // 0b00010000
        Unused = 1 << 5,           // 0b00100000
        Overflow = 1 << 6,         // 0b01000000
        Negative = 1 << 7,         // 0b10000000
    };

    // Flag methods
    void               SetFlags( u8 flag );
    void               ClearFlags( u8 flag );
    [[nodiscard]] auto IsFlagSet( u8 flag ) const -> bool;
    void               SetZeroAndNegativeFlags( u8 value );

    // LDA, LDX, and LDY helper
    void LoadRegister( u16 address, u8 &reg );
    void StoreRegister( u16 address, u8 reg ) const;

    /*
    ################################################################
    ||                                                            ||
    ||                      Addressing Modes                      ||
    ||                                                            ||
    ################################################################
    */
    auto IMP() -> u16; // Implied //NOLINT

    auto IMM() -> u16;  // Immediate
    auto ZPG() -> u16;  // Zero Page
    auto ZPGX() -> u16; // Zero Page X
    auto ZPGY() -> u16; // Zero Page Y
    auto ABS() -> u16;  // Absolute
    auto ABSX() -> u16; // Absolute X
    auto ABSY() -> u16; // Absolute Y
    auto IND() -> u16;  // Indirect
    auto INDX() -> u16; // Indirect X
    auto INDY() -> u16; // Indirect Y
    auto REL() -> u16;  // Relative

    /*
    ################################################################
    ||                                                            ||
    ||                        Instructions                        ||
    ||                                                            ||
    ################################################################
      */
    void LDA( u16 address );
    void LDX( u16 address );
    void LDY( u16 address );
    void STA( u16 address );
    void STX( u16 address );
    void STY( u16 address );

};

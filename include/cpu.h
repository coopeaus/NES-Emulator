#pragma once

#include <array>
#include <cstdint>
#include <string>

// Aliases for integer types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;

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

    // public cpu methods
    void               Reset();
    [[nodiscard]] u8   Fetch();
    void               Tick();
    [[nodiscard]] auto Read( u16 address ) const -> u8;
    void               Write( u16 address, u8 data ) const;

    // Debugging
    std::string LogLineAtPC( bool verbose = true );

  private:
    friend class CPUTestFixture; // Sometimes used for testing private methods

    Bus *_bus;         // Pointer to the Bus class
    bool _imp = false; // Implicit addressing mode flag

    // Registers
    u16 _pc = 0x0000;       // Program counter (PC)
    u8  _a = 0x00;          // Accumulator register (A)
    u8  _x = 0x00;          // X register
    u8  _y = 0x00;          // Y register
    u8  _s = 0xFD;          // Stack pointer (SP)
    u8  _p = 0x00 | Unused; // Status register (P), per the specs, the unused flag should always be set
    u64 _cycles = 0;        // Number of cycles

    // Instruction data
    struct InstructionData
    {
        std::string name;                        // Instruction mnemonic (e.g. LDA, STA)
        void ( CPU::*instructionMethod )( u16 ); // Pointer to the instruction helper method
        u16 ( CPU::*addressingModeMethod )();    // Pointer to the address mode helper method
        u8 cycles;                               // Number of cycles the instruction takes
        u8 bytes;                                // Number of bytes the instruction takes
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

    // Branch helper
    void BranchOnStatus( u16 offsetAddress, u8 flag, bool isSet );

    // Compare helper
    void CompareAddressWithRegister( u16 address, u8 reg );

    // Push/Pop helper
    void               StackPush( u8 value );
    [[nodiscard]] auto StackPop() -> u8;

    /*
    ################################################################
    ||                                                            ||
    ||                      Addressing Modes                      ||
    ||                                                            ||
    ################################################################
    */
    auto IMP() -> u16;  // Implicit
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

    // NOP
    void NOP( u16 address );

    // Load/Store
    void LDA( u16 address );
    void LDX( u16 address );
    void LDY( u16 address );
    void STA( u16 address );
    void STX( u16 address );
    void STY( u16 address );

    // Arithmetic
    void ADC( u16 address );
    void SBC( u16 address );
    void INC( u16 address );
    void DEC( u16 address );
    void INX( u16 address );
    void INY( u16 address );
    void DEX( u16 address );
    void DEY( u16 address );

    // Clear/Set flags
    void CLC( u16 address );
    void CLI( u16 address );
    void CLD( u16 address );
    void CLV( u16 address );
    void SEC( u16 address );
    void SED( u16 address );
    void SEI( u16 address );

    // Branch
    void BPL( u16 address );
    void BMI( u16 address );
    void BVC( u16 address );
    void BVS( u16 address );
    void BCC( u16 address );
    void BCS( u16 address );
    void BNE( u16 address );
    void BEQ( u16 address );

    // Compare
    void CMP( u16 address );
    void CPX( u16 address );
    void CPY( u16 address );

    // Shift
    void ASL( u16 address );
    void LSR( u16 address );
    void ROL( u16 address );
    void ROR( u16 address );

    // Stack
    void PHA( u16 address );
    void PHP( u16 address );
    void PLA( u16 address );
    void PLP( u16 address );
    void TSX( u16 address );
    void TXS( u16 address );

    // Jumps
    void JMP( u16 address );
    void JSR( u16 address );
    void RTS( u16 address );
    void RTI( u16 address );
    void BRK( u16 address );

    // Bitwise
    void AND( u16 address );
    void EOR( u16 address );
    void ORA( u16 address );
    void BIT( u16 address );

    // Transfer
    void TAX( u16 address );
    void TXA( u16 address );
    void TAY( u16 address );
    void TYA( u16 address );

    /*
    ################################################
    ||                                            ||
    ||               Illegal Opcodes              ||
    ||                                            ||
    ################################################
    */
    void JAM( u16 address );
    void SLO( u16 address );
    void SAX( u16 address );
    void LXA( u16 address );
    void LAX( u16 address );
    void ARR( u16 address );
    void ALR( u16 address );
    void RRA( u16 address );
    void SRE( u16 address );
    void RLA( u16 address );
    void DCP( u16 address );
    void ISC( u16 address );
    void ANC( u16 address );
};

// cpu.cpp

#include "cpu.h"
#include "bus.h"
#include <cstdint>
#include <iostream>

CPU::CPU( Bus *bus ) : _bus( bus ), _opcodeTable{}
{
    /*
    ################################################################
    ||                                                            ||
    ||                      Set Opcodes here                      ||
    ||                                                            ||
    ################################################################
    */

    // LDA
    _opcodeTable[0xA9] = InstructionData{ "LDA_Immediate", &CPU::LDA, &CPU::IMM, 2 };
    _opcodeTable[0xA5] = InstructionData{ "LDA_ZeroPage", &CPU::LDA, &CPU::ZPG, 3 };
    _opcodeTable[0xB5] = InstructionData{ "LDA_ZeroPageX", &CPU::LDA, &CPU::ZPGX, 4 };
    _opcodeTable[0xAD] = InstructionData{ "LDA_Absolute", &CPU::LDA, &CPU::ABS, 4 };
    _opcodeTable[0xBD] = InstructionData{ "LDA_AbsoluteX", &CPU::LDA, &CPU::ABSX, 4 };
    _opcodeTable[0xB9] = InstructionData{ "LDA_AbsoluteY", &CPU::LDA, &CPU::ABSY, 4 };
    _opcodeTable[0xA1] = InstructionData{ "LDA_IndirectX", &CPU::LDA, &CPU::INDX, 6 };
    _opcodeTable[0xB1] = InstructionData{ "LDA_IndirectY", &CPU::LDA, &CPU::INDY, 5 };

    // LDX
    _opcodeTable[0xA2] = InstructionData{ "LDX_Immediate", &CPU::LDX, &CPU::IMM, 2 };
    _opcodeTable[0xA6] = InstructionData{ "LDX_ZeroPage", &CPU::LDX, &CPU::ZPG, 3 };
    _opcodeTable[0xB6] = InstructionData{ "LDX_ZeroPageY", &CPU::LDX, &CPU::ZPGY, 4 };
    _opcodeTable[0xAE] = InstructionData{ "LDX_Absolute", &CPU::LDX, &CPU::ABS, 4 };
    _opcodeTable[0xBE] = InstructionData{ "LDX_AbsoluteY", &CPU::LDX, &CPU::ABSY, 4 };

    // LDY
    _opcodeTable[0xA0] = InstructionData{ "LDY_Immediate", &CPU::LDY, &CPU::IMM, 2 };
    _opcodeTable[0xA4] = InstructionData{ "LDY_ZeroPage", &CPU::LDY, &CPU::ZPG, 3 };
    _opcodeTable[0xB4] = InstructionData{ "LDY_ZeroPageX", &CPU::LDY, &CPU::ZPGX, 4 };
    _opcodeTable[0xAC] = InstructionData{ "LDY_Absolute", &CPU::LDY, &CPU::ABS, 4 };
    _opcodeTable[0xBC] = InstructionData{ "LDY_AbsoluteX", &CPU::LDY, &CPU::ABSX, 4 };

    // STA
    _opcodeTable[0x85] = InstructionData{ "STA_ZeroPage", &CPU::STA, &CPU::ZPG, 3 };
    _opcodeTable[0x95] = InstructionData{ "STA_ZeroPageX", &CPU::STA, &CPU::ZPGX, 4 };
    _opcodeTable[0x8D] = InstructionData{ "STA_Absolute", &CPU::STA, &CPU::ABS, 4 };
    _opcodeTable[0x9D] = InstructionData{ "STA_AbsoluteX", &CPU::STA, &CPU::ABSX, 5, false };
    _opcodeTable[0x99] = InstructionData{ "STA_AbsoluteY", &CPU::STA, &CPU::ABSY, 5, false };
    _opcodeTable[0x81] = InstructionData{ "STA_IndirectX", &CPU::STA, &CPU::INDX, 6, false };
    _opcodeTable[0x91] = InstructionData{ "STA_IndirectY", &CPU::STA, &CPU::INDY, 6, false };

    // STX
    _opcodeTable[0x86] = InstructionData{ "STX_ZeroPage", &CPU::STX, &CPU::ZPG, 3 };
    _opcodeTable[0x96] = InstructionData{ "STX_ZeroPageY", &CPU::STX, &CPU::ZPGY, 4 };
    _opcodeTable[0x8E] = InstructionData{ "STX_Absolute", &CPU::STX, &CPU::ABS, 4 };

    // STY
    _opcodeTable[0x84] = InstructionData{ "STY_ZeroPage", &CPU::STY, &CPU::ZPG, 3 };
    _opcodeTable[0x94] = InstructionData{ "STY_ZeroPageX", &CPU::STY, &CPU::ZPGX, 4 };
    _opcodeTable[0x8C] = InstructionData{ "STY_Absolute", &CPU::STY, &CPU::ABS, 4 };

    // Transfer Instructions
    _opcodeTable[0xAA] = InstructionData{ "TAX", &CPU::TAX, &CPU::IMP, 2 };
    _opcodeTable[0x8A] = InstructionData{ "TXA", &CPU::TXA, &CPU::IMP, 2 };
    _opcodeTable[0xA8] = InstructionData{ "TAY", &CPU::TAY, &CPU::IMP, 2 };
    _opcodeTable[0x98] = InstructionData{ "TYA", &CPU::TYA, &CPU::IMP, 2 };


};

// Getters
[[nodiscard]] u8  CPU::GetAccumulator() const { return _a; }
[[nodiscard]] u8  CPU::GetXRegister() const { return _x; }
[[nodiscard]] u8  CPU::GetYRegister() const { return _y; }
[[nodiscard]] u8  CPU::GetStatusRegister() const { return _p; }
[[nodiscard]] u16 CPU::GetProgramCounter() const { return _pc; }
[[nodiscard]] u8  CPU::GetStackPointer() const { return _s; }
[[nodiscard]] u64 CPU::GetCycles() const { return _cycles; }

// Setters
void CPU::SetAccumulator( u8 value ) { _a = value; }
void CPU::SetXRegister( u8 value ) { _x = value; }
void CPU::SetYRegister( u8 value ) { _y = value; }
void CPU::SetStatusRegister( u8 value ) { _p = value; }
void CPU::SetProgramCounter( u16 value ) { _pc = value; }
void CPU::SetStackPointer( u8 value ) { _s = value; }
void CPU::SetCycles( u64 value ) { _cycles = value; }

/*
################################################################
||                                                            ||
||                        CPU Methods                         ||
||                                                            ||
################################################################
*/

// Pass off reads and writes to the bus
auto CPU::Read( u16 address ) const -> u8 { return _bus->Read( address ); }
void CPU::Write( u16 address, u8 data ) const { _bus->Write( address, data ); }

u8 CPU::Fetch()
{
    // Read the current PC location and increment it
    return Read( _pc++ );
}

/**
 * @brief Executes a single CPU cycle.
 *
 * This function fetches the next opcode from memory, decodes it using the opcode table,
 * and executes that instruction. It also adds the number of cycles the instruction
 * takes to the total cycle count.
 *
 * If the opcode is invalid, an error message is printed to stderr.
 */
void CPU::Tick()
{

    // Fetch the next opcode and increment the program counter
    u8 const opcode = Fetch();

    // Decode the opcode
    auto const &instruction = _opcodeTable[opcode];
    auto        instruction_handler = instruction.instructionMethod;
    auto        addressing_mode_handler = instruction.addressingModeMethod;

    if ( instruction_handler != nullptr && addressing_mode_handler != nullptr )
    {
        // Set the page cross penalty for the current instruction
        // Used in addressing modes: ABSX, ABSY, INDY
        _currentPageCrossPenalty = instruction.pageCrossPenalty;

        // Calculate the address using the addressing mode
        u16 const address = ( this->*addressing_mode_handler )();

        // Execute the instruction fetched from the opcode table
        ( this->*instruction_handler )( address );

        // Add the number of cycles the instruction takes
        _cycles += instruction.cycles;
    }
    else
    {
        // Houston, we have a problem. No opcode was found.
        std::cerr << "Bad opcode: " << std::hex << static_cast<int>( opcode ) << '\n';
    }
}

void CPU::Reset()
{
    _a = 0x00;
    _x = 0x00;
    _y = 0x00;
    _s = 0xFD;
    _p = 0x00 | Unused;
    _cycles = 0;

    // The program counter is usually read from the reset vector of a game, which is
    // located at 0xFFFC and 0xFFFD. If no cartridge, we'll assume these values are
    // initialized to 0x00
    _pc = Read( 0xFFFD ) << 8 | Read( 0xFFFC );
}

/*
################################################################
||                                                            ||
||                      Addressing Modes                      ||
||                                                            ||
################################################################
*/

auto CPU::IMP() -> u16
{
    /*
     * @brief Implied addressing mode
     * This addressing mode does not require an operand
     */
    return 0;
}

auto CPU::IMM() -> u16
{
    /*
     * @brief Returns address of the next byte in memory (the operand itself)
     * The operand is a part of the instruction
     * The program counter is incremented to point to the operand
     */
    return _pc++;
}

auto CPU::ZPG() -> u16
{
    /*
     * @brief Zero Page addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF).
     * The value of the next byte is the address in the zero page.
     */
    return Read( _pc++ ) & 0x00FF;
}

auto CPU::ZPGX() -> u16
{
    /*
     * @brief Zero Page X addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF) + X register
     * The value of the next byte is the address in the zero page.
     */
    return ( Read( _pc++ ) + _x ) & 0x00FF;
}

auto CPU::ZPGY() -> u16
{
    /*
     * @brief Zero Page Y addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF) + Y register
     * The value of the next byte is the address in the zero page.
     */
    return ( Read( _pc++ ) + _y ) & 0x00FF;
}

auto CPU::ABS() -> u16
{
    /*
     * @brief Absolute addressing mode
     * Constructs a 16-bit address from the next two bytes
     */
    u16 const low = Read( _pc++ );
    u16 const high = Read( _pc++ );
    return ( high << 8 ) | low;
}

auto CPU::ABSX() -> u16
{
    /*
     * @brief Absolute X addressing mode
     * Constructs a 16-bit address from the next two bytes and adds the X register to the final
     * address
     */
    u16 const low = Read( _pc++ );
    u16 const high = Read( _pc++ );
    u16 const address = ( high << 8 ) | low;
    u16 const final_address = address + _x;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: ASL, ROL, LSR, ROR, STA, DEC, INC
    if ( _currentPageCrossPenalty && ( final_address & 0xFF00 ) != ( address & 0xFF00 ) )
    {
        _cycles++;
    }

    return final_address;
}

auto CPU::ABSY() -> u16
{
    /*
     * @brief Absolute Y addressing mode
     * Constructs a 16-bit address from the next two bytes and adds the Y register to the final
     * address
     */
    u16 const low = Read( _pc++ );
    u16 const high = Read( _pc++ );
    u16 const address = ( high << 8 ) | low;
    u16 const final_address = address + _y;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: STA
    if ( _currentPageCrossPenalty && ( final_address & 0xFF00 ) != ( address & 0xFF00 ) )
    {
        _cycles++;
    }

    return final_address;
}

auto CPU::IND() -> u16
{
    /*
     * @brief Indirect addressing mode
     * This mode implements pointers.
     * The pointer address will be read from the next two bytes
     * The returning value is the address stored at the pointer address
     * There's a hardware bug that prevents the address from crossing a page boundary
     */

    u16 const ptr_low = Read( _pc++ );
    u16 const ptr_high = Read( _pc++ );
    u16 const ptr = ( ptr_high << 8 ) | ptr_low;

    u8 const address_low = Read( ptr );
    u8       address_high; // NOLINT

    // 6502 Bug: If the pointer address wraps around a page boundary (e.g. 0x01FF),
    // the CPU reads the low byte from 0x01FF and the high byte from the start of
    // the same page (0x0100) instead of the start of the next page (0x0200).
    if ( ptr_low == 0xFF )
    {
        address_high = Read( ptr & 0xFF00 );
    }
    else
    {
        address_high = Read( ptr + 1 );
    }

    return ( address_high << 8 ) | address_low;
}

auto CPU::INDX() -> u16
{
    /*
     * @brief Indirect X addressing mode
     * The next two bytes are a zero-page address
     * X register is added to the zero-page address to get the pointer address
     * Final address is the value stored at the POINTER address
     */
    u8 const  zero_page_address = ( Read( _pc++ ) + _x ) & 0x00FF;
    u16 const ptr_low = Read( zero_page_address );
    u16 const ptr_high = Read( ( zero_page_address + 1 ) & 0x00FF );
    return ( ptr_high << 8 ) | ptr_low;
}

auto CPU::INDY() -> u16
{
    /*
     * @brief Indirect Y addressing mode
     * The next byte is a zero-page address
     * The value stored at the zero-page address is the pointer address
     * The value in the Y register is added to the FINAL address
     */
    u16 const zero_page_address = Read( _pc++ );
    u16 const ptr_low = Read( zero_page_address );
    u16 const ptr_high = Read( ( zero_page_address + 1 ) & 0x00FF );

    u16 const address = ( ( ptr_high << 8 ) | ptr_low ) + _y;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: STA
    if ( _currentPageCrossPenalty && ( address & 0xFF00 ) != ( ptr_high << 8 ) )
    {
        _cycles++;
    }
    return address;
}

auto CPU::REL() -> u16
{
    /*
     * @brief Relative addressing mode
     * The next byte is a signed offset
     * Sets the program counter between -128 and +127 bytes from the current location
     */
    using s8 = std::int8_t;
    s8 const  offset = static_cast<s8>( Read( _pc ) );
    u16 const address = _pc + offset;
    _pc++;
    return address;
}

/*
################################################################
||                                                            ||
||                    Instruction Helpers                     ||
||                                                            ||
################################################################
*/

void CPU::LoadRegister( u16 address, u8 &reg )
{
    /*
     * @brief It loads a register with a value from memory
     * Used by LDA, LDX, and LDY instructions
     */
    u8 const value = Read( address );
    reg = value;

    // Set zero and negative flags
    SetZeroAndNegativeFlags( value );
};

void CPU::StoreRegister( u16 address, u8 reg )
{
    /*
     * @brief It stores a register value in memory
     * Used by STA, STX, and STY instructions
     */
    Write( address, reg );
};

void CPU::SetFlags( const u8 flag )
{
    /*
     * @brief set one or more flag bits through bitwise OR with the status register
     *
     * Used by the SEC, SED, and SEI instructions to set one or more flag bits through
     * bitwise OR with the status register.
     *
     * Usage:
     * SetFlags( Status::Carry ); // Set one flag
     * SetFlags( Status::Carry | Status::Zero ); // Set multiple flags
     */
    _p |= flag;
}
void CPU::ClearFlags( const u8 flag )
{
    /* Clear Flags
     * @brief clear one or more flag bits through bitwise AND of the complement (inverted) flag with
     * the status register
     *
     * Used by the CLC, CLD, and CLI instructions to clear one or more flag bits through
     * bitwise AND of the complement (inverted) flag with the status register.
     *
     * Usage:
     * ClearFlags( Status::Carry ); // Clear one flag
     * ClearFlags( Status::Carry | Status::Zero ); // Clear multiple flags
     */
    _p &= ~flag;
}
bool CPU::IsFlagSet( const u8 flag ) const
{
    /* @brief Utility function to check if a given status is set in the status register
     *
     * Usage:
     * if ( IsFlagSet( Status::Carry ) )
     * {
     *   // Do something
     * }
     * if ( IsFlagSet( Status::Carry | Status::Zero ) )
     * {
     *   // Do something
     * }
     */
    return ( _p & flag ) == flag;
}

void CPU::SetZeroAndNegativeFlags( u8 value )
{
    /*
     * @brief Sets zero flag if value == 0, or negative flag if value is negative (bit 7 is set)
     */

    // Clear zero and negative flags
    ClearFlags( Status::Zero | Status::Negative );

    // Set zero flag if value is zero
    if ( value == 0 )
    {
        SetFlags( Status::Zero );
    }

    // Set negative flag if bit 7 is set
    if ( ( value & 0b10000000 ) != 0 )
    {
        SetFlags( Status::Negative );
    }
}

/*
################################################################
||                                                            ||
||                        Instructions                        ||
||                                                            ||
################################################################
* These functions should take no arguments and return no values.
* All complicated or reusable logic should be defined in the helper
* methods.
*/

void CPU::LDA( u16 address )
{
    /*
     * @brief Load Accumulator with Memory
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * LDA Immediate: A9(2)
     * LDA Zero Page: A5(3)
     * LDA Zero Page X: B5(4)
     * LDA Absolute: AD(4)
     * LDA Absolute X: BD(4+)
     * LDA Absolute Y: B9(4+)
     * LDA Indirect X: A1(6)
     * LDA Indirect Y: B1(5+)
     */

    LoadRegister( address, _a );
}

void CPU::LDX( u16 address )
{
    /*
     * @brief Load X Register with Memory
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * LDX Immediate: A2(2)
     * LDX Zero Page: A6(3)
     * LDX Zero Page Y: B6(4)
     * LDX Absolute: AE(4)
     * LDX Absolute Y: BE(4+)
     */
    LoadRegister( address, _x );
}

void CPU::LDY( u16 address )
{
    /*
     * @brief Load Y Register with Memory
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * LDY Immediate: A0(2)
     * LDY Zero Page: A4(3)
     * LDY Zero Page X: B4(4)
     * LDY Absolute: AC(4)
     * LDY Absolute X: BC(4+)
     */
    LoadRegister( address, _y );
}

void CPU::STA( u16 address )
{
    /*
     * @brief Store Accumulator in Memory
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * STA Zero Page: 85(3)
     * STA Zero Page X: 95(4)
     * STA Absolute: 8D(4)
     * STA Absolute X: 9D(5)
     * STA Absolute Y: 99(5)
     * STA Indirect X: 81(6)
     * STA Indirect Y: 91(6)
     */
    StoreRegister( address, _a );
}

void CPU::STX( u16 address )
{
    /*
     * @brief Store X Register in Memory
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * STX Zero Page: 86(3)
     * STX Zero Page Y: 96(4)
     * STX Absolute: 8E(4)
     */
    StoreRegister( address, _x );
}

void CPU::STY( u16 address )
{
    /*
     * @brief Store Y Register in Memory
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * STY Zero Page: 84(3)
     * STY Zero Page X: 94(4)
     * STY Absolute: 8C(4)
     */
    StoreRegister( address, _y );
}

void CPU::TAX( u16 address )
{
    /*
     * @brief Transfer Accumulator to X Register
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * TAX Implied: AA(2)
     */
    _x = _a;
    SetZeroAndNegativeFlags( _x );
}

void CPU::TXA( u16 address )
{
    /*
     * @brief Transfer X Register to Accumulator
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * TXA Implied: 8A(2)
     */
    _a = _x;
    SetZeroAndNegativeFlags( _a );
}

void CPU::TAY( u16 address )
{
    /*
     * @brief Transfer Accumulator to Y Register
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * TAY Implied: A8(2)
     */
    _y = _a;
    SetZeroAndNegativeFlags( _y );
}

void CPU::TYA( u16 address )
{
    /*
     * @brief Transfer Y Register to Accumulator
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * TYA Implied: 98(2)
     */
    _a = _y;
    SetZeroAndNegativeFlags( _a );
}

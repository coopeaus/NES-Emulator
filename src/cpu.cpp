// cpu.cpp

#include "bus.h"
#include "cpu.h"
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

    // ADC
    _opcodeTable[0x69] = InstructionData{ "ADC_Immediate", &CPU::ADC, &CPU::IMM, 2 };
    _opcodeTable[0x65] = InstructionData{ "ADC_ZeroPage", &CPU::ADC, &CPU::ZPG, 3 };
    _opcodeTable[0x75] = InstructionData{ "ADC_ZeroPageX", &CPU::ADC, &CPU::ZPGX, 4 };
    _opcodeTable[0x6D] = InstructionData{ "ADC_Absolute", &CPU::ADC, &CPU::ABS, 4 };
    _opcodeTable[0x7D] = InstructionData{ "ADC_AbsoluteX", &CPU::ADC, &CPU::ABSX, 4 };
    _opcodeTable[0x79] = InstructionData{ "ADC_AbsoluteY", &CPU::ADC, &CPU::ABSY, 4 };
    _opcodeTable[0x61] = InstructionData{ "ADC_IndirectX", &CPU::ADC, &CPU::INDX, 6 };
    _opcodeTable[0x71] = InstructionData{ "ADC_IndirectY", &CPU::ADC, &CPU::INDY, 5 };

    // SBC
    _opcodeTable[0xE9] = InstructionData{ "SBC_Immediate", &CPU::SBC, &CPU::IMM, 2 };
    _opcodeTable[0xE5] = InstructionData{ "SBC_ZeroPage", &CPU::SBC, &CPU::ZPG, 3 };
    _opcodeTable[0xF5] = InstructionData{ "SBC_ZeroPageX", &CPU::SBC, &CPU::ZPGX, 4 };
    _opcodeTable[0xED] = InstructionData{ "SBC_Absolute", &CPU::SBC, &CPU::ABS, 4 };
    _opcodeTable[0xFD] = InstructionData{ "SBC_AbsoluteX", &CPU::SBC, &CPU::ABSX, 4 };
    _opcodeTable[0xF9] = InstructionData{ "SBC_AbsoluteY", &CPU::SBC, &CPU::ABSY, 4 };
    _opcodeTable[0xE1] = InstructionData{ "SBC_IndirectX", &CPU::SBC, &CPU::INDX, 6 };
    _opcodeTable[0xF1] = InstructionData{ "SBC_IndirectY", &CPU::SBC, &CPU::INDY, 5 };

    // INC
    _opcodeTable[0xE6] = InstructionData{ "INC_ZeroPage", &CPU::INC, &CPU::ZPG, 5 };
    _opcodeTable[0xF6] = InstructionData{ "INC_ZeroPageX", &CPU::INC, &CPU::ZPGX, 6 };
    _opcodeTable[0xEE] = InstructionData{ "INC_Absolute", &CPU::INC, &CPU::ABS, 6 };
    _opcodeTable[0xFE] = InstructionData{ "INC_AbsoluteX", &CPU::INC, &CPU::ABSX, 7, false };

    // DEC
    _opcodeTable[0xC6] = InstructionData{ "DEC_ZeroPage", &CPU::DEC, &CPU::ZPG, 5 };
    _opcodeTable[0xD6] = InstructionData{ "DEC_ZeroPageX", &CPU::DEC, &CPU::ZPGX, 6 };
    _opcodeTable[0xCE] = InstructionData{ "DEC_Absolute", &CPU::DEC, &CPU::ABS, 6 };
    _opcodeTable[0xDE] = InstructionData{ "DEC_AbsoluteX", &CPU::DEC, &CPU::ABSX, 7, false };

    // INX, INY, DEX, DEY
    _opcodeTable[0xE8] = InstructionData{ "INX", &CPU::INX, &CPU::IMP, 2 };
    _opcodeTable[0xC8] = InstructionData{ "INY", &CPU::INY, &CPU::IMP, 2 };
    _opcodeTable[0xCA] = InstructionData{ "DEX", &CPU::DEX, &CPU::IMP, 2 };
    _opcodeTable[0x88] = InstructionData{ "DEY", &CPU::DEY, &CPU::IMP, 2 };

    // CLC
    _opcodeTable[0x18] = InstructionData{ "CLC_Implied", &CPU::CLC, &CPU::IMP, 2 };
    _opcodeTable[0x58] = InstructionData{ "CLI_Implied", &CPU::CLI, &CPU::IMP, 2 };
    _opcodeTable[0xD8] = InstructionData{ "CLD_Implied", &CPU::CLD, &CPU::IMP, 2 };
    _opcodeTable[0xB8] = InstructionData{ "CLV_Implied", &CPU::CLV, &CPU::IMP, 2 };

    _opcodeTable[0x38] = InstructionData{ "SEC_Implied", &CPU::SEC, &CPU::IMP, 2 };
    _opcodeTable[0x78] = InstructionData{ "SEI_Implied", &CPU::SEI, &CPU::IMP, 2 };
    _opcodeTable[0xF8] = InstructionData{ "SED_Implied", &CPU::SED, &CPU::IMP, 2 };

    // Branch
    _opcodeTable[0x10] = InstructionData{ "BPL_Relative", &CPU::BPL, &CPU::REL, 2 };
    _opcodeTable[0x30] = InstructionData{ "BMI_Relative", &CPU::BMI, &CPU::REL, 2 };
    _opcodeTable[0x50] = InstructionData{ "BVC_Relative", &CPU::BVC, &CPU::REL, 2 };
    _opcodeTable[0x70] = InstructionData{ "BVS_Relative", &CPU::BVS, &CPU::REL, 2 };
    _opcodeTable[0x90] = InstructionData{ "BCC_Relative", &CPU::BCC, &CPU::REL, 2 };
    _opcodeTable[0xB0] = InstructionData{ "BCS_Relative", &CPU::BCS, &CPU::REL, 2 };
    _opcodeTable[0xD0] = InstructionData{ "BNE_Relative", &CPU::BNE, &CPU::REL, 2 };
    _opcodeTable[0xF0] = InstructionData{ "BEQ_Relative", &CPU::BEQ, &CPU::REL, 2 };

    // CMP, CPX, CPY
    _opcodeTable[0xC9] = InstructionData{ "CMP_Immediate", &CPU::CMP, &CPU::IMM, 2 };
    _opcodeTable[0xC5] = InstructionData{ "CMP_ZeroPage", &CPU::CMP, &CPU::ZPG, 3 };
    _opcodeTable[0xD5] = InstructionData{ "CMP_ZeroPageX", &CPU::CMP, &CPU::ZPGX, 4 };
    _opcodeTable[0xCD] = InstructionData{ "CMP_Absolute", &CPU::CMP, &CPU::ABS, 4 };
    _opcodeTable[0xDD] = InstructionData{ "CMP_AbsoluteX", &CPU::CMP, &CPU::ABSX, 4 };
    _opcodeTable[0xD9] = InstructionData{ "CMP_AbsoluteY", &CPU::CMP, &CPU::ABSY, 4 };
    _opcodeTable[0xC1] = InstructionData{ "CMP_IndirectX", &CPU::CMP, &CPU::INDX, 6 };
    _opcodeTable[0xD1] = InstructionData{ "CMP_IndirectY", &CPU::CMP, &CPU::INDY, 5 };
    _opcodeTable[0xE0] = InstructionData{ "CPX_Immediate", &CPU::CPX, &CPU::IMM, 2 };
    _opcodeTable[0xE4] = InstructionData{ "CPX_ZeroPage", &CPU::CPX, &CPU::ZPG, 3 };
    _opcodeTable[0xEC] = InstructionData{ "CPX_Absolute", &CPU::CPX, &CPU::ABS, 4 };
    _opcodeTable[0xC0] = InstructionData{ "CPY_Immediate", &CPU::CPY, &CPU::IMM, 2 };
    _opcodeTable[0xC4] = InstructionData{ "CPY_ZeroPage", &CPU::CPY, &CPU::ZPG, 3 };
    _opcodeTable[0xCC] = InstructionData{ "CPY_Absolute", &CPU::CPY, &CPU::ABS, 4 };
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
    s8 const  offset = static_cast<s8>( Read( _pc++ ) );
    u16 const address = _pc + offset;
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

void CPU::StoreRegister( u16 address, u8 reg ) const
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
     * @brief clear one or more flag bits through bitwise AND of the complement (inverted) flag
     * with the status register
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

void CPU::BranchOnStatus( u16 offsetAddress, u8 flag, bool isSet )
{
    /* @brief Branch if status flag is set or clear
     *
     * Used by branch instructions to branch if a status flag is set or clear.
     *
     * Usage:
     * BranchOnStatus( Status::Carry, true ); // Branch if carry flag is set
     * BranchOnStatus( Status::Zero, false ); // Branch if zero flag is clear
     */

    bool const will_branch = ( _p & flag ) == flag;

    // Path will branch
    if ( will_branch == isSet )
    {
        // Store previous program counter value, used to check boundary crossing
        u16 const prev_pc = _pc;

        // Set _pc to the offset address, calculated by REL addressing mode
        _pc = offsetAddress;

        // +1 cyles because we're taking a branch
        _cycles++;

        // Add another cycle if page boundary is crossed
        if ( ( _pc & 0xFF00 ) != ( prev_pc & 0xFF00 ) )
        {
            _cycles += 1;
        }
    }
    // Path will not branch, nothing to do
}

void CPU::CompareAddressWithRegister( u16 address, u8 reg )
{
    /*
     * @brief Compare a value in memory with a register
     * Used by CMP, CPX, and CPY instructions
     */

    u8 const value = Read( address );

    // Set the zero flag if the values are equal
    ( reg == value ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

    // Set negative flag if the result is negative,
    // i.e. the sign bit is set
    ( ( reg - value ) & 0b10000000 ) != 0 ? SetFlags( Status::Negative )
                                          : ClearFlags( Status::Negative );

    // Set the carry flag if the reg >= value
    ( reg >= value ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
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

void CPU::STA( const u16 address ) // NOLINT
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

void CPU::STX( const u16 address ) // NOLINT
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

void CPU::STY( const u16 address ) // NOLINT
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

void CPU::ADC( u16 address )
{
    /*
     * @brief Add Memory to Accumulator with Carry
     * N Z C I D V
     * + + + - - +
     * Usage and cycles:
     * ADC Immediate: 69(2)
     * ADC Zero Page: 65(3)
     * ADC Zero Page X: 75(4)
     * ADC Absolute: 6D(4)
     * ADC Absolute X: 7D(4+)
     * ADC Absolute Y: 79(4+)
     * ADC Indirect X: 61(6)
     * ADC Indirect Y: 71(5+)
     */
    u8 const value = Read( address );

    // Store the sum in a 16-bit variable to check for overflow
    u8 const  carry = IsFlagSet( Status::Carry ) ? 1 : 0;
    u16 const sum = _a + value + carry;

    // Set the carry flag if sum > 255
    // this means that there will be an overflow
    ( sum > 0xFF ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

    // If the lower part of sum is zero, set the zero flag
    ( ( sum & 0xFF ) == 0 ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

    // Signed overflow is set if the sign bit is different in the accumulator and the result
    // e.g.
    // 1000 0001 + // << Accumulator: -127
    // 1000 0001   // << Value: -127
    // ---------
    // 0000 0010   // << Sum: 2. Sign bit is different, result is positive but should be
    // negative
    u8 const accumulator_sign_bit = _a & 0b10000000;
    u8 const value_sign_bit = value & 0b10000000;
    u8 const sum_sign_bit = sum & 0b10000000;
    ( accumulator_sign_bit == value_sign_bit && accumulator_sign_bit != sum_sign_bit )
        ? SetFlags( Status::Overflow )
        : ClearFlags( Status::Overflow );

    // If bit 7 is set, set the negative flag
    ( sum & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Store the lower byte of the sum in the accumulator
    _a = sum & 0xFF;
}

void CPU::SBC( u16 address )
{
    /* @brief Subtract Memory from Accumulator with Borrow
     * N Z C I D V
     * + + + - - +
     * Usage and cycles:
     * SBC Immediate: E9(2)
     * SBC Zero Page: E5(3)
     * SBC Zero Page X: F5(4)
     * SBC Absolute: ED(4)
     * SBC Absolute X: FD(4+)
     * SBC Absolute Y: F9(4+)
     * SBC Indirect X: E1(6)
     * SBC Indirect Y: F1(5+)
     */

    u8 const value = Read( address );

    // Store diff in a 16-bit variable to check for overflow
    u8 const  carry = IsFlagSet( Status::Carry ) ? 0 : 1;
    u16 const diff = _a - value - carry;

    // Carry flag exists in the high byte?
    ( diff < 0x100 ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

    // If the lower part of diff is zero, set the zero flag
    ( ( diff & 0xFF ) == 0 ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

    // Signed overflow is set if the sign bit is different in the accumulator and the result
    // e.g.
    // 0000 0001 - // << Accumulator: 1
    // 0000 0010   // << Value: 2
    // ---------
    // 1111 1111   // << Diff: 127. Sign bit is different
    u8 const accumulator_sign_bit = _a & 0b10000000;
    u8 const value_sign_bit = value & 0b10000000;
    u8 const diff_sign_bit = diff & 0b10000000;
    ( accumulator_sign_bit != value_sign_bit && accumulator_sign_bit != diff_sign_bit )
        ? SetFlags( Status::Overflow )
        : ClearFlags( Status::Overflow );

    // If bit 7 is set, set the negative flag
    ( diff & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Store the lower byte of the diff in the accumulator
    _a = diff & 0xFF;
}

void CPU::INC( u16 address )
{
    /*
     * @brief Increment Memory by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INC Zero Page: E6(5)
     * INC Zero Page X: F6(6)
     * INC Absolute: EE(6)
     * INC Absolute X: FE(7)
     */
    u8 const value = Read( address );
    u8 const result = value + 1;
    SetZeroAndNegativeFlags( result );
    Write( address, result );
}

void CPU::INX( u16 address )
{
    /*
     * @brief Increment X Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INX: E8(2)
     */
    (void) address;
    _x++;
    SetZeroAndNegativeFlags( _x );
}

void CPU::INY( u16 address )
{
    /*
     * @brief Increment Y Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INY: C8(2)
     */
    (void) address;
    _y++;
    SetZeroAndNegativeFlags( _y );
}

void CPU::DEC( u16 address )
{
    /*
     * @brief Decrement Memory by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEC Zero Page: C6(5)
     * DEC Zero Page X: D6(6)
     * DEC Absolute: CE(6)
     * DEC Absolute X: DE(7)
     */
    u8 const value = Read( address );
    u8 const result = value - 1;
    SetZeroAndNegativeFlags( result );
    Write( address, result );
}

void CPU::DEX( u16 address )
{
    /*
     * @brief Decrement X Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEX: CA(2)
     */
    (void) address;
    _x--;
    SetZeroAndNegativeFlags( _x );
}

void CPU::DEY( u16 address )
{
    /*
     * @brief Decrement Y Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEY: 88(2)
     */
    (void) address;
    _y--;
    SetZeroAndNegativeFlags( _y );
}

void CPU::CLC( const u16 address )
{
    /* @brief Clear Carry Flag
     * N Z C I D V
     * - - 0 - - -
     *   Usage and cycles:
     *   CLC: 18(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::Carry );
}

void CPU::CLI( const u16 address )
{
    /* @brief Clear Interrupt Disable
     * N Z C I D V
     * - - - 0 - -
     *   Usage and cycles:
     *   CLI: 58(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::InterruptDisable );
}
void CPU::CLD( const u16 address )
{
    /* @brief Clear Decimal Mode
     * N Z C I D V
     * - - - - 0 -
     *   Usage and cycles:
     *   CLD: D8(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::Decimal );
}
void CPU::CLV( const u16 address )
{
    /* @brief Clear Overflow Flag
     * N Z C I D V
     * - - - - - 0
     *   Usage and cycles:
     *   CLV: B8(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::Overflow );
}

void CPU::SEC( const u16 address )
{
    /* @brief Set Carry Flag
     * N Z C I D V
     * - - 1 - - -
     *   Usage and cycles:
     *   SEC: 38(2)
     */
    (void) address;
    CPU::SetFlags( CPU::Carry );
}

void CPU::SED( const u16 address )
{
    /* @brief Set Decimal Flag
     * N Z C I D V
     * - - - - 1 -
     *   Usage and cycles:
     *   SED: F8(2)
     */
    (void) address;
    CPU::SetFlags( CPU::Decimal );
}

void CPU::SEI( const u16 address )
{
    /* @brief Set Interrupt Disable
     * N Z C I D V
     * - - - 1 - -
     *   Usage and cycles:
     *   SEI: 78(2)
     */
    (void) address;
    CPU::SetFlags( CPU::InterruptDisable );
}

void CPU::BPL( const u16 address )
{
    /* @brief Branch if Positive
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BPL: 10(2+)
     */
    BranchOnStatus( address, Status::Negative, false );
}

void CPU::BMI( const u16 address )
{
    /* @brief Branch if Minus
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BMI: 30(2+)
     */
    BranchOnStatus( address, Status::Negative, true );
}

void CPU::BVC( const u16 address )
{
    /* @brief Branch if Overflow Clear
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BVC: 50(2+)
     */
    BranchOnStatus( address, Status::Overflow, false );
}

void CPU::BVS( const u16 address )
{
    /* @brief Branch if Overflow Set
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BVS: 70(2+)
     */
    BranchOnStatus( address, Status::Overflow, true );
}

void CPU::BCC( const u16 address )
{
    /* @brief Branch if Carry Clear
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BCC: 90(2+)
     */
    BranchOnStatus( address, Status::Carry, false );
}

void CPU::BCS( const u16 address )
{
    /* @brief Branch if Carry Set
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BCS: B0(2+)
     */
    BranchOnStatus( address, Status::Carry, true );
}

void CPU::BNE( const u16 address )
{
    /* @brief Branch if Not Equal
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BNE: D0(2+)
     */
    BranchOnStatus( address, Status::Zero, false );
}

void CPU::BEQ( const u16 address )
{
    /* @brief Branch if Equal
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BEQ: F0(2+)
     */
    BranchOnStatus( address, Status::Zero, true );
}

void CPU::CMP( u16 address )
{
    /* @brief Compare Memory and Accumulator
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CMP Immediate: C9(2)
     *   CMP Zero Page: C5(3)
     *   CMP Zero Page X: D5(4)
     *   CMP Absolute: CD(4)
     *   CMP Absolute X: DD(4+)
     *   CMP Absolute Y: D9(4+)
     *   CMP Indirect X: C1(6)
     *   CMP Indirect Y: D1(5+)
     */
    CompareAddressWithRegister( address, _a );
}

void CPU::CPX( u16 address )
{
    /* @brief Compare Memory and X Register
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CPX Immediate: E0(2)
     *   CPX Zero Page: E4(3)
     *   CPX Absolute: EC(4)
     */
    CompareAddressWithRegister( address, _x );
}

void CPU::CPY( u16 address )
{
    /* @brief Compare Memory and Y Register
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CPY Immediate: C0(2)
     *   CPY Zero Page: C4(3)
     *   CPY Absolute: CC(4)
     */
    CompareAddressWithRegister( address, _y );
}

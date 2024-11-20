// cpu.cpp

#include "cpu.h"
#include "bus.h"
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
    _opcodeTable[0xA9] = InstructionData{ "LDA", &CPU::LDA, &CPU::IMM, 2 };
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
void CPU::Write( u16 address, u8 data ) { _bus->Write( address, data ); }

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
        // Execute the instruction fetched from the opcode table
        ( this->*instruction_handler )();

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
||                    Instruction Helpers                     ||
||                                                            ||
################################################################
*/

void CPU::LoadRegister( u16 address, u8 &reg )
{
    /*
      Used by all loading instructions (LDA, LDX, LDY)
      It loads a register with a value from memory
    */
    u8 const value = Read( address );
    reg = value;

    // Set zero and negative flags
    SetZeroAndNegativeFlags( value );
};

void CPU::SetFlags( const u8 flag )
{
    /* Set Flags
      Used by the SEC, SED, and SEI instructions to set one or more flag bits through
      bitwise OR with the status register.

      Usage:
      SetFlags( Status::Carry ); // Set one flag
      SetFlags( Status::Carry | Status::Zero ); // Set multiple flags
    */
    _p |= flag;
}
void CPU::ClearFlags( const u8 flag )
{
    /* Clear Flags
      Used by the CLC, CLD, and CLI instructions to clear one or more flag bits through
      bitwise AND of the complement (inverted) flag with the status register.

      Usage:
      ClearFlags( Status::Carry ); // Clear one flag
      ClearFlags( Status::Carry | Status::Zero ); // Clear multiple flags
    */
    _p &= ~flag;
}
bool CPU::IsFlagSet( const u8 flag ) const
{
    /* Utility function to check if a given status is set in the status register

      Usage:
      if ( IsFlagSet( Status::Carry ) )
      {
        // Do something
      }
      if ( IsFlagSet( Status::Carry | Status::Zero ) )
      {
        // Do something
      }
     */
    return ( _p & flag ) == flag;
}

void CPU::SetZeroAndNegativeFlags( u8 value )
{
    /*
    Sets zero flag if value == 0, or negative flag if value is negative (bit 7 is set)
    Used by a lot of instructions
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
||                      Addressing Modes                      ||
||                                                            ||
################################################################
*/
u16 CPU::IMM()
{
    /*
      Immediate
      Returns address of the next byte in memory (the operand itself)
      The operand is a part of the instruction
      The program counter is incremented to point to the operand
    */
    return _pc++;
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

void CPU::LDA()
{
    /*
      Load Accumulator
      This covers LDA with all addressing modes
      Loads a byte of memory into the accumulator setting the zero and negative flags as
      appropriate.
    */
    auto      addressing_mode = _opcodeTable[0xA9].addressingModeMethod;
    u16 const address = ( this->*addressing_mode )();
    LoadRegister( address, _a );
}

void CPU::LDX()
{
    /*
      Load X Register
      This covers LDX with all addressing modes
      Loads a byte of memory into the X register setting the zero and negative flags as
      appropriate.
    */
    auto      addressing_mode = _opcodeTable[0xA2].addressingModeMethod;
    u16 const address = ( this->*addressing_mode )();
    LoadRegister( address, _x );
}

void CPU::LDY()
{
    /*
      Load Y Register
      This covers LDY with all addressing modes
      Loads a byte of memory into the Y register setting the zero and negative flags as
      appropriate.
    */
    auto      addressing_mode = _opcodeTable[0xA0].addressingModeMethod;
    u16 const address = ( this->*addressing_mode )();
    LoadRegister( address, _y );
}

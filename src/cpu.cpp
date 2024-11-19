// cpu.cpp

#include "bus.h"
#include "cpu.h"
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

// TODO: Fetch, Decode, Execute

// Pass off reads and writes to the bus
auto CPU::Read( u16 address ) const -> u8 { return _bus->Read( address ); }
void CPU::Write( u16 address, u8 data ) { _bus->Write( address, data ); }

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
################################################################
||                                                            ||
||                      Addressing Modes                      ||
||                                                            ||
################################################################
*/
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

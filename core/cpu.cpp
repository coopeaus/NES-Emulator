// cpu.cpp
/*
   Most logic is in cpu.h. Things that interact with the bus will go here though.
*/

#include "bus.h"
#include "cpu.h"
#include "cpu-types.h"
#include "global-types.h"
#include "utils.h"
#include <string>
#include <stdexcept>

/*
################################################
||                                            ||
||                Debug Methods               ||
||                                            ||
################################################
*/

std::string CPU::LogLineAtPC( bool verbose ) // NOLINT
{
  /*
   * @brief Disassembles the instruction at the current program counter
   * Useful to understand what the current instruction is doing
   */
  std::string output;

  u8 const           opcode = Read( pc );
  std::string const &name = gInstructionNames.at( opcode );
  std::string const &addrMode = gAddressingModes.at( opcode );

  // Program counter address
  // i.e. FFFF
  output += utils::toHex( pc, 4 ) + " ";

  if ( verbose ) {
    output += "  ";
    // Hex instruction
    // i.e. 4C F5 C5, this is the hex instruction
    u8 const    bytes = gInstructionBytes.at( opcode );
    std::string hexInstruction;
    for ( u8 i = 0; i < bytes; i++ ) {
      hexInstruction += utils::toHex( Read( pc + i ), 2 ) + ' ';
    }

    // formatting, the instruction hex_instruction will be 9 characters long, with space padding to
    // the right. This makes sure the hex line is the same length for all instructions
    hexInstruction += std::string( 9 - ( bytes * 3 ), ' ' );
    output += hexInstruction;
  }

  // If name starts with a "*", it is an illegal opcode
  ( name[0] == '*' ) ? output += "*" + name.substr( 1 ) + " " : output += name + " ";

  // Addressing mode and operand

  std::string assemblyStr;
  u8          value = 0x00;
  u8          low = 0x00;
  u8          high = 0x00;
  if ( addrMode == "IMP" ) {
    // Nothing to prefix
  } else if ( addrMode == "IMM" ) {
    value = Read( pc + 1 );
    assemblyStr += "#$" + utils::toHex( value, 2 );
  } else if ( addrMode == "ZPG" || addrMode == "ZPGX" || addrMode == "ZPGY" ) {
    value = Read( pc + 1 );
    assemblyStr += "$" + utils::toHex( value, 2 );

    ( addrMode == "ZPGX" ) ? assemblyStr += ", X" : ( addrMode == "ZPGY" ) ? assemblyStr += ", Y" : assemblyStr += "";
  } else if ( addrMode == "ABS" || addrMode == "ABSX" || addrMode == "ABSY" ) {
    low = Read( pc + 1 );
    high = Read( pc + 2 );
    u16 const address = ( high << 8 ) | low;

    assemblyStr += "$" + utils::toHex( address, 4 );
    ( addrMode == "ABSX" ) ? assemblyStr += ", X" : ( addrMode == "ABSY" ) ? assemblyStr += ", Y" : assemblyStr += "";
  } else if ( addrMode == "IND" ) {
    low = Read( pc + 1 );
    high = Read( pc + 2 );
    u16 const address = ( high << 8 ) | low;
    assemblyStr += "($" + utils::toHex( address, 4 ) + ")";
  } else if ( addrMode == "INDX" || addrMode == "INDY" ) {
    value = Read( pc + 1 );
    ( addrMode == "INDX" ) ? assemblyStr += "($" + utils::toHex( value, 2 ) + ", X)"
                           : assemblyStr += "($" + utils::toHex( value, 2 ) + "), Y";
  } else if ( addrMode == "REL" ) {
    value = Read( pc + 1 );
    s8 const  offset = static_cast<s8>( value );
    u16 const address = pc + 2 + offset;

    assemblyStr += "$" + utils::toHex( value, 2 ) + " [$" + utils::toHex( address, 4 ) + "]";
  } else {
    // Houston.. yet again
    throw std::runtime_error( "Unknown addressing mode: " + addrMode );
  }

  // Pad the assembly string with spaces, for fixed length
  if ( verbose ) {
    output += assemblyStr + std::string( 15 - assemblyStr.size(), ' ' );
  }

  // Add more log info
  std::string registersStr;
  // Format
  // a: 00 x: 00 y: 00 s: FD
  registersStr += "a: " + utils::toHex( a, 2 ) + " ";
  registersStr += "x: " + utils::toHex( x, 2 ) + " ";
  registersStr += "y: " + utils::toHex( y, 2 ) + " ";
  registersStr += "s: " + utils::toHex( s, 2 ) + " ";

  // status register
  // Will return a formatted status string
  // p: hex value, status string (NV-BDIZC). Letter present is flag set, dash is flag unset
  std::string statusStr;
  statusStr += "p: " + utils::toHex( p, 2 ) + " ";

  std::string statusFlags = "NV-BDIZC";
  std::string statusFlagsLower = "nv--dizc";
  std::string statusFlagsStr;
  for ( int i = 7; i >= 0; i-- ) {
    statusFlagsStr += ( p & ( 1 << i ) ) != 0 ? statusFlags[7 - i] : statusFlagsLower[7 - i];
  }
  statusStr += statusFlagsStr;

  // Combine to the output string
  output += registersStr + statusStr;

  // Scanline num (V)
  if ( verbose ) {
    std::string const scanlineStr = std::to_string( bus->ppu.scanline );
    // std::string scanlinestradjusted = std::string( 4 - scanlinestr.size(), ' ' );
    output += "  V: " + scanlineStr;

    // PPU cycles (H), pad for 3 characters + space
    u16 const   ppuCycles = bus->ppu.cycle;
    std::string ppuCyclesStr = std::to_string( ppuCycles );
    ppuCyclesStr += std::string( 4 - ppuCyclesStr.size(), ' ' );
    output += "  H: " + ppuCyclesStr; // PPU cycle

    // cycle count
    output += "  Cycle: " + std::to_string( cycles );
  }

  return output;
}

/*
################################################################
||                                                            ||
||                        CPU Methods                         ||
||                                                            ||
################################################################
*/

// Pass off reads and writes to the bus
auto CPU::Read( u16 address, bool debugMode ) const -> u8
{
  return bus->Read( address, debugMode );
}
void CPU::Write( u16 address, u8 data ) const
{
  bus->Write( address, data );
}

// Read with cycle spend
auto CPU::ReadAndTick( u16 address ) -> u8
{
  if ( address == 0x2002 ) {
    SetReading2002( true );
  }
  Tick();
  u8 const data = Read( address );
  return data;
}

// Write and spend a cycle
auto CPU::WriteAndTick( u16 address, u8 data ) -> void
{
  Tick();

  // Writing to PPUCTRL, PPUMASK, PPUSCROLL, and PPUADDR is ignored until after cycle ~29658
  if ( !bus->IsTestMode() && ( address == 0x2000 || address == 0x2001 || address == 0x2005 || address == 0x2006 ) ) {
    if ( cycles < 29658 ) {
      return;
    }
  }
  Write( address, data );
}

u8 CPU::Fetch()
{

  // Read the current PC location and increment it

  u8 const opcode = ReadAndTick( pc++ );
  return opcode;
}

void CPU::Tick()
{
  // Increment the cycle count
  cycles++;
  bus->ppu.Tick();
  bus->ppu.Tick();

  // Match mesen trace log, place logger here.
  if ( mesenFormatTraceEnabled && !didMesenTrace ) {
    pc--;
    AddMesenTracelog( LogLineAtPC( true ) );
    pc++;
    didMesenTrace = true;
  }

  bus->ppu.Tick();
}

void CPU::Reset()
{
  a = 0x00;
  x = 0x00;
  y = 0x00;
  s = 0xFD;
  p = 0x00 | Unused;

  // The program counter is usually read from the reset vector of a game, which is
  // located at 0xFFFC and 0xFFFD. If no cartridge, we'll assume 0x00 for both
  pc = Read( 0xFFFD ) << 8 | Read( 0xFFFC );

  // Add 7 cycles
  if ( !bus->IsTestMode() ) {

    for ( u8 i = 0; i < 7; i++ ) {
      Tick();
    }
  } else {
    cycles = 0;
  }
}

void CPU::DecodeExecute()
{

  /**
   * @brief Decode and execute an instruction
   *
   * This function fetches the next opcode from memory, decodes it using the opcode table,
   * and executes that instruction.
   *
   */

  if ( traceEnabled ) {
    AddTraceLog( LogLineAtPC( true ) );
  }

  didMesenTrace = false;

  // Fetch the next opcode and increment the program counter
  opcode = Fetch();
  auto const &instruction = opcodeTable.at( opcode );
  auto        instructionHandler = instruction.handler;
  auto        addressingModeHandler = instruction.addrMode;

  // Set the page cross penalty for the current instruction
  // Used in addressing modes: ABSX, ABSY, INDY
  pageCrossPenalty = isPageCrossPenalty( opcode );

  // Write / modify instructions use a dummy read before writing, so
  // we should set a flag for those
  writeModify = isWriteModify( opcode );

  // Set current instr mnemonic globally
  instructionName = gInstructionNames.at( opcode );

  // Set current address mode string globally
  addrMode = gAddressingModes.at( opcode );

  // Calculate the address using the addressing mode
  u16 const address = ( this->*addressingModeHandler )();

  // Execute the instruction fetched from the opcode table
  ( this->*instructionHandler )( address );

  // Reset flags
  writeModify = false;
  didMesenTrace = false;
}

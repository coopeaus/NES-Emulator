#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <fmt/base.h>
#include <string>
#include "global-types.h"
// NOLINTBEGIN
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/deque.hpp>
// NOLINTEND

// Forward declaration for reads and writes
class Bus;

class CPU
{
public:
  explicit CPU( Bus *bus ) : bus( bus )
  {

    // clang-format off
#define Imp( op )  Instruction { &CPU::op, &CPU::IMP }
#define Imm( op )  Instruction { &CPU::op, &CPU::IMM }
#define Zpg( op )  Instruction { &CPU::op, &CPU::ZPG }
#define ZpgX( op ) Instruction { &CPU::op, &CPU::ZPGX }
#define ZpgY( op ) Instruction { &CPU::op, &CPU::ZPGY }
#define Abs( op )  Instruction { &CPU::op, &CPU::ABS }
#define AbsX( op ) Instruction { &CPU::op, &CPU::ABSX }
#define AbsY( op ) Instruction { &CPU::op, &CPU::ABSY }
#define Ind( op )  Instruction { &CPU::op, &CPU::IND }
#define IndX( op ) Instruction { &CPU::op, &CPU::INDX }
#define IndY( op ) Instruction { &CPU::op, &CPU::INDY }
#define Rel( op )  Instruction { &CPU::op, &CPU::REL }

        opcodeTable = {
          //0        1          2          3          4           5          6          7          8         9          A         B          C           D          E          F
    /*0*/ Imp(BRK),  IndX(ORA), Imp(JAM),  IndX(SLO), Zpg(NOP2),  Zpg(ORA),  Zpg(ASL),  Zpg(SLO),  Imp(PHP), Imm(ORA),  Imp(ASL), Imm(ANC),  Abs(NOP2),  Abs(ORA),  Abs(ASL),  Abs(SLO),
    /*1*/ Rel(BPL),  IndY(ORA), Imp(JAM),  IndY(SLO), ZpgX(NOP2), ZpgX(ORA), ZpgX(ASL), ZpgX(SLO), Imp(CLC), AbsY(ORA), Imp(NOP), AbsY(SLO), AbsX(NOP2), AbsX(ORA), AbsX(ASL), AbsX(SLO),
    /*2*/ Abs(JSR),  IndX(AND), Imp(JAM),  IndX(RLA), Zpg(BIT),   Zpg(AND),  Zpg(ROL),  Zpg(RLA),  Imp(PLP), Imm(AND),  Imp(ROL), Imm(ANC),  Abs(BIT),   Abs(AND),  Abs(ROL),  Abs(RLA),
    /*3*/ Rel(BMI),  IndY(AND), Imp(JAM),  IndY(RLA), ZpgX(NOP2), ZpgX(AND), ZpgX(ROL), ZpgX(RLA), Imp(SEC), AbsY(AND), Imp(NOP), AbsY(RLA), AbsX(NOP2), AbsX(AND), AbsX(ROL), AbsX(RLA),
    /*4*/ Imp(RTI),  IndX(EOR), Imp(JAM),  IndX(SRE), Zpg(NOP2),  Zpg(EOR),  Zpg(LSR),  Zpg(SRE),  Imp(PHA), Imm(EOR),  Imp(LSR), Imm(ALR),  Abs(JMP),   Abs(EOR),  Abs(LSR),  Abs(SRE),
    /*5*/ Rel(BVC),  IndY(EOR), Imp(JAM),  IndY(SRE), ZpgX(NOP2), ZpgX(EOR), ZpgX(LSR), ZpgX(SRE), Imp(CLI), AbsY(EOR), Imp(NOP), AbsY(SRE), AbsX(NOP2), AbsX(EOR), AbsX(LSR), AbsX(SRE),
    /*6*/ Imp(RTS),  IndX(ADC), Imp(JAM),  IndX(RRA), Zpg(NOP2),  Zpg(ADC),  Zpg(ROR),  Zpg(RRA),  Imp(PLA), Imm(ADC),  Imp(ROR), Imm(ARR),  Ind(JMP),   Abs(ADC),  Abs(ROR),  Abs(RRA),
    /*7*/ Rel(BVS),  IndY(ADC), Imp(JAM),  IndY(RRA), ZpgX(NOP2), ZpgX(ADC), ZpgX(ROR), ZpgX(RRA), Imp(SEI), AbsY(ADC), Imp(NOP), AbsY(RRA), AbsX(NOP2), AbsX(ADC), AbsX(ROR), AbsX(RRA),
    /*8*/ Imm(NOP2), IndX(STA), Imm(NOP2), IndX(SAX), Zpg(STY),   Zpg(STA),  Zpg(STX),  Zpg(SAX),  Imp(DEY), Imm(NOP2), Imp(TXA), Imm(ANE), Abs(STY),   Abs(STA),  Abs(STX),  Abs(SAX),
    /*9*/ Rel(BCC),  IndY(STA), Imp(JAM),  IndY(SHA), ZpgX(STY),  ZpgX(STA), ZpgY(STX), ZpgY(SAX), Imp(TYA), AbsY(STA), Imp(TXS), AbsY(TAS), AbsX(SHY),  AbsX(STA), AbsY(SHX), AbsY(SHA),
    /*A*/ Imm(LDY),  IndX(LDA), Imm(LDX),  IndX(LAX), Zpg(LDY),   Zpg(LDA),  Zpg(LDX),  Zpg(LAX),  Imp(TAY), Imm(LDA),  Imp(TAX), Imm(ATX),  Abs(LDY),   Abs(LDA),  Abs(LDX),  Abs(LAX),
    /*B*/ Rel(BCS),  IndY(LDA), Imp(JAM),  IndY(LAX), ZpgX(LDY),  ZpgX(LDA), ZpgY(LDX), ZpgY(LAX), Imp(CLV), AbsY(LDA), Imp(TSX), AbsY(LAS), AbsX(LDY),  AbsX(LDA), AbsY(LDX), AbsY(LAX),
    /*C*/ Imm(CPY),  IndX(CMP), Imm(NOP2), IndX(DCP), Zpg(CPY),   Zpg(CMP),  Zpg(DEC),  Zpg(DCP),  Imp(INY), Imm(CMP),  Imp(DEX), Imm(SBX),  Abs(CPY),   Abs(CMP),  Abs(DEC),  Abs(DCP),
    /*D*/ Rel(BNE),  IndY(CMP), Imp(JAM),  IndY(DCP), ZpgX(NOP2), ZpgX(CMP), ZpgX(DEC), ZpgX(DCP), Imp(CLD), AbsY(CMP), Imp(NOP), AbsY(DCP), AbsX(NOP2), AbsX(CMP), AbsX(DEC), AbsX(DCP),
    /*E*/ Imm(CPX),  IndX(SBC), Imm(NOP2), IndX(ISC), Zpg(CPX),   Zpg(SBC),  Zpg(INC),  Zpg(ISC),  Imp(INX), Imm(SBC),  Imp(NOP), Imm(SBC),  Abs(CPX),   Abs(SBC),  Abs(INC),  Abs(ISC),
    /*F*/ Rel(BEQ),  IndY(SBC), Imp(JAM),  IndY(ISC), ZpgX(NOP2), ZpgX(SBC), ZpgX(INC), ZpgX(ISC), Imp(SED), AbsY(SBC), Imp(NOP), AbsY(ISC), AbsX(NOP2), AbsX(SBC), AbsX(INC), AbsX(ISC)
        };
    // clang-format on
  }

  /*
  ################################
  ||          Serialize         ||
  ################################
  */
  template <class Archive> void serialize( Archive &ar ) // NOLINT
  {
    ar( pc, a, x, y, s, p, cycles, didVblank, pageCrossPenalty, writeModify, reading2002, instructionName, addrMode,
        opcode, isTestMode, traceEnabled, mesenFormatTraceEnabled, didMesenTrace, traceLog, mesenFormatTraceLog );
  }

  /*
  ################################
  ||           Getters          ||
  ################################
  */
  u8   GetAccumulator() const { return a; }
  u8   GetXRegister() const { return x; }
  u8   GetYRegister() const { return y; }
  u8   GetStatusRegister() const { return p; }
  u16  GetProgramCounter() const { return pc; }
  u8   GetStackPointer() const { return s; }
  u64  GetCycles() const { return cycles; }
  bool IsReading2002() const { return reading2002; }

  // status getters
  u8 GetCarryFlag() const { return ( p & Carry ) >> 0; }
  u8 GetZeroFlag() const { return ( p & Zero ) >> 1; }
  u8 GetInterruptDisableFlag() const { return ( p & InterruptDisable ) >> 2; }
  u8 GetDecimalFlag() const { return ( p & Decimal ) >> 3; }
  u8 GetBreakFlag() const { return ( p & Break ) >> 4; }
  u8 GetOverflowFlag() const { return ( p & Overflow ) >> 6; }
  u8 GetNegativeFlag() const { return ( p & Negative ) >> 7; }

  /*
  ################################
  ||           Setters          ||
  ################################
  */
  void SetAccumulator( u8 value ) { a = value; }
  void SetXRegister( u8 value ) { x = value; }
  void SetYRegister( u8 value ) { y = value; }
  void SetStatusRegister( u8 value ) { p = value; }
  void SetProgramCounter( u16 value ) { pc = value; }
  void SetStackPointer( u8 value ) { s = value; }
  void SetCycles( u64 value ) { cycles = value; }
  void SetReading2002( bool value ) { reading2002 = value; };

  // status setters
  void SetCarryFlag( bool value ) { value ? SetFlags( Carry ) : ClearFlags( Carry ); }
  void SetZeroFlag( bool value ) { value ? SetFlags( Zero ) : ClearFlags( Zero ); }
  void SetInterruptDisableFlag( bool value ) { value ? SetFlags( InterruptDisable ) : ClearFlags( InterruptDisable ); }
  void SetDecimalFlag( bool value ) { value ? SetFlags( Decimal ) : ClearFlags( Decimal ); }
  void SetBreakFlag( bool value ) { value ? SetFlags( Break ) : ClearFlags( Break ); }
  void SetOverflowFlag( bool value ) { value ? SetFlags( Overflow ) : ClearFlags( Overflow ); }
  void SetNegativeFlag( bool value ) { value ? SetFlags( Negative ) : ClearFlags( Negative ); }

  /*
  ################################
  ||         CPU Methods        ||
  ################################
  */
  void Reset();
  u8   Fetch();
  void DecodeExecute();
  void Tick();
  auto Read( u16 address, bool debugMode = false ) const -> u8;
  auto ReadByte( u16 address ) -> u8;
  void Write( u16 address, u8 data ) const;
  void WriteByte( u16 address, u8 data );

  void NMI()
  {
    /* @details: Non-maskable Interrupt, called by the PPU during the VBlank period.
     * It interrupts whatever the CPU is doing at its current cycle to go update the PPU.
     * Uses 7 cycles, cannot be disabled.
     */
    // 1) Two dummy cycles (hardware reads the same PC twice, discarding the data)
    Tick();
    Tick();

    // 2) Push PC high, then PC low
    StackPush( ( pc >> 8 ) & 0xFF );
    StackPush( pc & 0xFF );

    // 3) Push status register with B=0; bit 5 (Unused) = 1
    u8 const pushedStatus = ( p & ~Break ) | Unused;
    StackPush( pushedStatus );

    // 4) Fetch low byte of NMI vector ($FFFA)
    u8 const low = ReadByte( 0xFFFA );

    // 5) Set I flag
    SetFlags( InterruptDisable );

    // 6) Fetch high byte of NMI vector ($FFFB)
    u8 const high = ReadByte( 0xFFFB );

    // 7) Update PC
    pc = static_cast<u16>( high ) << 8 | low;
  }

  void IRQ()
  {
    /* @brief: IRQ, can be called when interrupt disable is turned off.
     * Uses 7 cycles
     */
    if ( ( p & InterruptDisable ) != 0 ) {
      return;
    }
    Tick();
    Tick();
    StackPush( ( pc >> 8 ) & 0xFF );
    StackPush( pc & 0xFF );
    u8 const pushedStatus = ( p & ~Break ) | Unused;
    StackPush( pushedStatus );
    u8 const low = ReadByte( 0xFFFE );
    SetFlags( InterruptDisable );
    u8 const high = ReadByte( 0xFFFF );
    pc = static_cast<u16>( high ) << 8 | low;
  }

  /*
  ################################
  ||        Debug Methods       ||
  ################################
  */
  std::string             LogLineAtPC( bool verbose = true );
  std::deque<std::string> GetTracelog() const { return traceLog; }
  std::deque<std::string> GetMesenFormatTracelog() const { return mesenFormatTraceLog; }
  void                    EnableTracelog()
  {
    traceEnabled = true;
    mesenFormatTraceEnabled = false;
  }
  void EnableMesenFormatTraceLog()
  {
    mesenFormatTraceEnabled = true;
    traceEnabled = false;
  }
  void DisableTracelog() { traceEnabled = false; }
  void DisableMesenFormatTraceLog() { mesenFormatTraceEnabled = false; }
  void EnableJsonTestMode() { isTestMode = true; }
  void DisableJsonTestMode() { isTestMode = false; }

  size_t traceSize = 100;
  size_t mesenTraceSize = 100;
  void   SetMesenTraceSize( int size ) { mesenTraceSize = size; }
  void   AddTraceLog( const std::string &log )
  {
    if ( traceEnabled ) {
      traceLog.push_back( log + "\n" );
      if ( traceLog.size() > traceSize ) {
        traceLog.pop_front();
      }
    }
  }
  void ClearTraceLog() { traceLog.clear(); }
  void AddMesenTracelog( const std::string &log )
  {
    if ( mesenFormatTraceEnabled ) {
      mesenFormatTraceLog.push_back( log + "\n" );
      if ( mesenFormatTraceLog.size() > mesenTraceSize ) {
        mesenFormatTraceLog.pop_front();
      }
    }
  }
  void ClearMesenTraceLog() { mesenFormatTraceLog.clear(); }

  /*
  ################################
  ||      Global Variables      ||
  ################################
  */
  enum Status : u8 {
    Carry = 1 << 0,            // 0b00000001
    Zero = 1 << 1,             // 0b00000010
    InterruptDisable = 1 << 2, // 0b00000100
    Decimal = 1 << 3,          // 0b00001000
    Break = 1 << 4,            // 0b00010000
    Unused = 1 << 5,           // 0b00100000
    Overflow = 1 << 6,         // 0b01000000
    Negative = 1 << 7,         // 0b10000000
  };

  /*
  ################################
  ||         Peripherals        ||
  ################################
  */
  Bus *bus;

  /*
  ################################
  ||          Registers         ||
  ################################
  */
  u16 pc = 0x0000;       // Program counter (PC)
  u8  a = 0x00;          // Accumulator register (A)
  u8  x = 0x00;          // X register
  u8  y = 0x00;          // Y register
  u8  s = 0xFD;          // Stack pointer (SP)
  u8  p = 0x00 | Unused; // Status register (P), per the specs, the unused flag should always be set
  u64 cycles = 0;        // Number of cycles

  /*
  ################################
  ||  Private Global Variables  ||
  ################################
  */
  bool        didVblank = false;
  bool        pageCrossPenalty = true;
  bool        writeModify = false;
  bool        reading2002 = false;
  std::string instructionName;
  std::string addrMode;
  u8          opcode = 0x00;

  /*
  ################################
  ||       Debug Variables      ||
  ################################
  */
  bool isTestMode = false;
  bool traceEnabled = false;
  bool mesenFormatTraceEnabled = false;
  bool didMesenTrace = false;

  std::deque<std::string> traceLog;
  std::deque<std::string> mesenFormatTraceLog;

  /*
  ################################
  ||        Opcode Table        ||
  ################################
  */
  struct Instruction {
    void ( CPU::*handler )( u16 ){}; // Pointer to the instruction helper method
    u16 ( CPU::*addrMode )(){};      // Pointer to the address mode helper method
  };

  // Opcode table
  std::array<Instruction, 256> opcodeTable;
  Instruction                  GetInstruction( u8 opcode ) { return opcodeTable[opcode]; }

  /*
  ################################################################
  ||                                                            ||
  ||                    Instruction Helpers                     ||
  ||                                                            ||
  ################################################################
  */

  void LoadRegister( u16 address, u8 &reg )
  {
    /*
     * @brief It loads a register with a value from memory
     * Used by LDA, LDX, and LDY instructions
     */
    u8 const value = ReadByte( address );
    reg = value;

    // Set zero and negative flags
    SetZeroAndNegativeFlags( value );
  };

  void StoreRegister( u16 address, u8 reg )
  {
    /*
     * @brief It stores a register value in memory
     * Used by STA, STX, and STY instructions
     */
    WriteByte( address, reg );
  };

  void SetFlags( const u8 flag )
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
    p |= flag;
  }
  void ClearFlags( const u8 flag )
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
    p &= ~flag;
  }
  bool IsFlagSet( const u8 flag ) const
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
    return ( p & flag ) == flag;
  }

  void SetZeroAndNegativeFlags( u8 value )
  {
    /*
     * @brief Sets zero flag if value == 0, or negative flag if value is negative (bit 7 is set)
     */

    // Clear zero and negative flags
    ClearFlags( Status::Zero | Status::Negative );

    // Set zero flag if value is zero
    if ( value == 0 ) {
      SetFlags( Status::Zero );
    }

    // Set negative flag if bit 7 is set
    if ( ( value & 0b10000000 ) != 0 ) {
      SetFlags( Status::Negative );
    }
  }

  void BranchOnStatus( u16 offsetAddress, u8 flag, bool isSet )
  {
    /* @brief Branch if status flag is set or clear
     *
     * Used by branch instructions to branch if a status flag is set or clear.
     *
     * Usage:
     * BranchOnStatus( Status::Carry, true ); // Branch if carry flag is set
     * BranchOnStatus( Status::Zero, false ); // Branch if zero flag is clear
     */

    bool const willBranch = ( p & flag ) == flag;

    // Path will branch
    if ( willBranch == isSet ) {
      // Store previous program counter value, used to check boundary crossing
      u16 const prevPc = pc;

      // Set pc to the offset address, calculated by REL addressing mode
      pc = offsetAddress;

      // +1 cycles because we're taking a branch
      Tick();

      // Add another cycle if page boundary is crossed
      if ( ( pc & 0xFF00 ) != ( prevPc & 0xFF00 ) ) {
        Tick();
      }
    }
    // Path will not branch, nothing to do
  }

  void CompareAddressWithRegister( u16 address, u8 reg )
  {
    /*
     * @brief Compare a value in memory with a register
     * Used by CMP, CPX, and CPY instructions
     */

    u8 value = 0;
    if ( instructionName == "*DCP" ) {
      value = Read( address ); // 0 cycles
    } else {
      value = ReadByte( address );
    }

    // Set the zero flag if the values are equal
    ( reg == value ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

    // Set negative flag if the result is negative,
    // i.e. the sign bit is set
    ( ( reg - value ) & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Set the carry flag if the reg >= value
    ( reg >= value ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
  }

  void StackPush( u8 value )
  {
    /*
     * @brief Push a value onto the stack
     * The stack pointer is decremented and the value is written to the stack
     * Stack addresses are between 0x0100 and 0x01FF
     */
    WriteByte( 0x0100 + s--, value );
  }

  u8 StackPop()
  {
    /*
     * @brief Pop a value from the stack
     * The stack pointer is incremented and the value is read from the stack
     * Stack addresses are between 0x0100 and 0x01FF
     */
    return ReadByte( 0x0100 + ++s );
  }

  /*
  ################################################################
  ||                                                            ||
  ||                      Addressing Modes                      ||
  ||                                                            ||
  ################################################################
  */

  auto IMP() -> u16
  {
    /*
     * @brief Implicit addressing mode
     * This mode does not require an operand
     */
    Tick();
    return 0;
  }

  auto IMM() -> u16
  {
    /*
     * @brief Returns address of the next byte in memory (the operand itself)
     * The operand is a part of the instruction
     * The program counter is incremented to point to the operand
     */
    return pc++;
  }

  auto ZPG() -> u16
  {
    /*
     * @brief Zero Page addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF).
     * The value of the next byte is the address in the zero page.
     */
    return ReadByte( pc++ ) & 0x00FF;
  }

  auto ZPGX() -> u16
  {
    /*
     * @brief Zero Page X addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF) + X register
     * The value of the next byte is the address in the zero page.
     */
    u8 const  zeroPageAddress = ReadByte( pc++ );
    u16 const finalAddress = ( zeroPageAddress + x ) & 0x00FF;
    Tick(); // Account for calculating the final address
    return finalAddress;
  }

  auto ZPGY() -> u16
  {
    /*
     * @brief Zero Page Y addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF) + Y register
     * The value of the next byte is the address in the zero page.
     */
    u8 const zeroPageAddress = ( ReadByte( pc++ ) + y ) & 0x00FF;

    if ( writeModify ) {
      Tick();
    }
    return zeroPageAddress;
  }

  auto ABS() -> u16
  {
    /*
     * @brief Absolute addressing mode
     * Constructs a 16-bit address from the next two bytes
     */
    u16 const low = ReadByte( pc++ );
    u16 const high = ReadByte( pc++ );
    return ( high << 8 ) | low;
  }

  auto ABSX() -> u16
  {
    /*
     * @brief Absolute X addressing mode
     * Constructs a 16-bit address from the next two bytes and adds the X register to the final
     * address
     */
    u16 const low = ReadByte( pc++ );
    u16 const high = ReadByte( pc++ );
    u16 const address = ( high << 8 ) | low;
    u16 const finalAddress = address + x;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: ASL, ROL, LSR, ROR, STA, DEC, INC
    if ( pageCrossPenalty && ( finalAddress & 0xFF00 ) != ( address & 0xFF00 ) ) {
      Tick();
    }

    if ( writeModify ) {
      // Dummy read, in preparation to overwrite the address
      Tick();
    }
    return finalAddress;
  }

  auto ABSY() -> u16
  {
    /*
     * @brief Absolute Y addressing mode
     * Constructs a 16-bit address from the next two bytes and adds the Y register to the final
     * address
     */
    u16 const low = ReadByte( pc++ );
    u16 const high = ReadByte( pc++ );
    u16 const address = ( high << 8 ) | low;
    u16 const finalAddress = address + y;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: STA
    if ( pageCrossPenalty && ( finalAddress & 0xFF00 ) != ( address & 0xFF00 ) ) {
      Tick();
    }
    if ( writeModify ) {
      // Dummy read, in preparation to overwrite the address
      Tick();
    }

    return finalAddress;
  }

  auto IND() -> u16
  {
    /*
     * @brief Indirect addressing mode
     * This mode implements pointers.
     * The pointer address will be read from the next two bytes
     * The returning value is the address stored at the pointer address
     * There's a hardware bug that prevents the address from crossing a page boundary
     */

    u16 const ptrLow = ReadByte( pc++ );
    u16 const ptrHigh = ReadByte( pc++ );
    u16 const ptr = ( ptrHigh << 8 ) | ptrLow;

    u8 const addressLow = ReadByte( ptr );
    u8       address_high; // NOLINT

    // 6502 Bug: If the pointer address wraps around a page boundary (e.g. 0x01FF),
    // the CPU reads the low byte from 0x01FF and the high byte from the start of
    // the same page (0x0100) instead of the start of the next page (0x0200).
    if ( ptrLow == 0xFF ) {
      address_high = ReadByte( ptr & 0xFF00 );
    } else {
      address_high = ReadByte( ptr + 1 );
    }

    return ( address_high << 8 ) | addressLow;
  }

  auto INDX() -> u16
  {
    /*
     * @brief Indirect X addressing mode
     * The next two bytes are a zero-page address
     * X register is added to the zero-page address to get the pointer address
     * Final address is the value stored at the POINTER address
     */
    Tick();                                                           // Account for operand fetch
    u8 const  zeroPageAddress = ( ReadByte( pc++ ) + x ) & 0x00FF;    // 1 cycle
    u16 const ptrLow = ReadByte( zeroPageAddress );                   // 1 cycle
    u16 const ptrHigh = ReadByte( ( zeroPageAddress + 1 ) & 0x00FF ); // 1 cycle
    return ( ptrHigh << 8 ) | ptrLow;
  }

  auto INDY() -> u16
  {
    /*
     * @brief Indirect Y addressing mode
     * The next byte is a zero-page address
     * The value stored at the zero-page address is the pointer address
     * The value in the Y register is added to the FINAL address
     */
    u16 const zeroPageAddress = ReadByte( pc++ );
    u16 const ptrLow = ReadByte( zeroPageAddress );
    u16 const ptrHigh = ReadByte( ( zeroPageAddress + 1 ) & 0x00FF );

    u16 const address = ( ( ptrHigh << 8 ) | ptrLow ) + y;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: STA
    if ( pageCrossPenalty && ( address & 0xFF00 ) != ( ptrHigh << 8 ) ) {
      Tick();
    }

    if ( writeModify ) {
      // Dummy read, in preparation to overwrite the address
      Tick();
    }
    return address;
  }

  auto REL() -> u16
  {
    /*
     * @brief Relative addressing mode
     * The next byte is a signed offset
     * Sets the program counter between -128 and +127 bytes from the current location
     */
    s8 const  offset = static_cast<s8>( ReadByte( pc++ ) );
    u16 const address = pc + offset;
    return address;
  }

  /*
  ################################################################
  ||                                                            ||
  ||                        Instructions                        ||
  ||                                                            ||
  ################################################################
    */

  void NOP( u16 address ) // NOLINT
  {
    /*
     * @brief No operation
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * NOP Implied: EA(2)
     *
     * --  Illegal  --
     * NOP Implied: 1A(2)
     * NOP Implied: 3A(2)
     * NOP Implied: 5A(2)
     * NOP Implied: 7A(2)
     * NOP Implied: DA(2)
     * NOP Implied: FA(2)
     * NOP Immediate: 80(2)
     * NOP Immediate: 82(2)
     * NOP Immediate: 89(2)
     * NOP Immediate: C2(2)
     * NOP Immediate: E2(2)
     * NOP Zero Page: 04(3)
     * NOP Zero Page: 44(3)
     * NOP Zero Page: 64(3)
     * NOP Zero Page X: 14(4)
     * NOP Zero Page X: 34(4)
     * NOP Zero Page X: 54(4)
     * NOP Zero Page X: 74(4)
     * NOP Zero Page X: D4(4)
     * NOP Zero Page X: F4(4)
     * NOP Absolute: 0C(4)
     * NOP Absolute: 1C(4)
     * NOP Absolute: 3C(4)
     * NOP Absolute: 5C(4)
     * NOP Absolute: 7C(4)
     * NOP Absolute: DC(4)
     * NOP Absolute: FC(4)
     */
    (void) address;
  }

  void LDA( u16 address )
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

    LoadRegister( address, a );
  }

  void LDX( u16 address )
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
    LoadRegister( address, x );
  }

  void LDY( u16 address )
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
    LoadRegister( address, y );
  }

  void STA( const u16 address ) // NOLINT
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
    StoreRegister( address, a );
  }

  void STX( const u16 address ) // NOLINT
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
    StoreRegister( address, x );
  }

  void STY( const u16 address ) // NOLINT
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
    StoreRegister( address, y );
  }

  void ADC( u16 address )
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
    u8 value = 0;

    if ( instructionName == "*RRA" ) {
      value = Read( address ); // No cycle spend
    } else {
      value = ReadByte( address );
    }

    // Store the sum in a 16-bit variable to check for overflow
    u8 const  carry = IsFlagSet( Status::Carry ) ? 1 : 0;
    u16 const sum = a + value + carry;

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
    u8 const accumulatorSignBit = a & 0b10000000;
    u8 const valueSignBit = value & 0b10000000;
    u8 const sumSignBit = sum & 0b10000000;
    ( accumulatorSignBit == valueSignBit && accumulatorSignBit != sumSignBit ) ? SetFlags( Status::Overflow )
                                                                               : ClearFlags( Status::Overflow );

    // If bit 7 is set, set the negative flag
    ( sum & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Store the lower byte of the sum in the accumulator
    a = sum & 0xFF;
  }

  void SBC( u16 address )
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
     *
     * --  Illegal  --
     * SBC Immediate: EB(2)
     */

    u8 value = 0;
    if ( instructionName == "*ISC" ) {
      value = Read( address ); // 0 cycles
    } else {
      value = ReadByte( address );
    }
    // u8 const value = ReadByte( address );

    // Store diff in a 16-bit variable to check for overflow
    u8 const  carry = IsFlagSet( Status::Carry ) ? 0 : 1;
    u16 const diff = a - value - carry;

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
    u8 const accumulatorSignBit = a & 0b10000000;
    u8 const valueSignBit = value & 0b10000000;
    u8 const diffSignBit = diff & 0b10000000;
    ( accumulatorSignBit != valueSignBit && accumulatorSignBit != diffSignBit ) ? SetFlags( Status::Overflow )
                                                                                : ClearFlags( Status::Overflow );

    // If bit 7 is set, set the negative flag
    ( diff & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Store the lower byte of the diff in the accumulator
    a = diff & 0xFF;
  }

  void INC( u16 address )
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
    u8 const value = ReadByte( address );
    Tick(); // Dummy write
    u8 const result = value + 1;
    SetZeroAndNegativeFlags( result );
    WriteByte( address, result );
  }

  void INX( u16 address )
  {
    /*
     * @brief Increment X Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INX: E8(2)
     */
    (void) address;
    x++;
    SetZeroAndNegativeFlags( x );
  }

  void INY( u16 address )
  {
    /*
     * @brief Increment Y Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INY: C8(2)
     */
    (void) address;
    y++;
    SetZeroAndNegativeFlags( y );
  }

  void DEC( u16 address )
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
    u8 const value = ReadByte( address );
    Tick(); // Dummy write
    u8 const result = value - 1;
    SetZeroAndNegativeFlags( result );
    WriteByte( address, result );
  }

  void DEX( u16 address )
  {
    /*
     * @brief Decrement X Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEX: CA(2)
     */
    (void) address;
    x--;
    SetZeroAndNegativeFlags( x );
  }

  void DEY( u16 address )
  {
    /*
     * @brief Decrement Y Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEY: 88(2)
     */
    (void) address;
    y--;
    SetZeroAndNegativeFlags( y );
  }

  void CLC( const u16 address )
  {
    /* @brief Clear Carry Flag
     * N Z C I D V
     * - - 0 - - -
     *   Usage and cycles:
     *   CLC: 18(2)
     */
    (void) address;
    ClearFlags( Carry );
  }

  void CLI( const u16 address )
  {
    /* @brief Clear Interrupt Disable
     * N Z C I D V
     * - - - 0 - -
     *   Usage and cycles:
     *   CLI: 58(2)
     */
    (void) address;
    ClearFlags( InterruptDisable );
  }
  void CLD( const u16 address )
  {
    /* @brief Clear Decimal Mode
     * N Z C I D V
     * - - - - 0 -
     *   Usage and cycles:
     *   CLD: D8(2)
     */
    (void) address;
    ClearFlags( Decimal );
  }
  void CLV( const u16 address )
  {
    /* @brief Clear Overflow Flag
     * N Z C I D V
     * - - - - - 0
     *   Usage and cycles:
     *   CLV: B8(2)
     */
    (void) address;
    ClearFlags( Overflow );
  }

  void SEC( const u16 address )
  {
    /* @brief Set Carry Flag
     * N Z C I D V
     * - - 1 - - -
     *   Usage and cycles:
     *   SEC: 38(2)
     */
    (void) address;
    SetFlags( Carry );
  }

  void SED( const u16 address )
  {
    /* @brief Set Decimal Flag
     * N Z C I D V
     * - - - - 1 -
     *   Usage and cycles:
     *   SED: F8(2)
     */
    (void) address;
    SetFlags( Decimal );
  }

  void SEI( const u16 address )
  {
    /* @brief Set Interrupt Disable
     * N Z C I D V
     * - - - 1 - -
     *   Usage and cycles:
     *   SEI: 78(2)
     */
    (void) address;
    SetFlags( InterruptDisable );
  }

  void BPL( const u16 address )
  {
    /* @brief Branch if Positive
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BPL: 10(2+)
     */
    BranchOnStatus( address, Status::Negative, false );
  }

  void BMI( const u16 address )
  {
    /* @brief Branch if Minus
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BMI: 30(2+)
     */
    BranchOnStatus( address, Status::Negative, true );
  }

  void BVC( const u16 address )
  {
    /* @brief Branch if Overflow Clear
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BVC: 50(2+)
     */
    BranchOnStatus( address, Status::Overflow, false );
  }

  void BVS( const u16 address )
  {
    /* @brief Branch if Overflow Set
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BVS: 70(2+)
     */
    BranchOnStatus( address, Status::Overflow, true );
  }

  void BCC( const u16 address )
  {
    /* @brief Branch if Carry Clear
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BCC: 90(2+)
     */
    BranchOnStatus( address, Status::Carry, false );
  }

  void BCS( const u16 address )
  {
    /* @brief Branch if Carry Set
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BCS: B0(2+)
     */
    BranchOnStatus( address, Status::Carry, true );
  }

  void BNE( const u16 address )
  {
    /* @brief Branch if Not Equal
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BNE: D0(2+)
     */
    BranchOnStatus( address, Status::Zero, false );
  }

  void BEQ( const u16 address )
  {
    /* @brief Branch if Equal
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BEQ: F0(2+)
     */
    BranchOnStatus( address, Status::Zero, true );
  }

  void CMP( u16 address )
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
    CompareAddressWithRegister( address, a );
  }

  void CPX( u16 address )
  {
    /* @brief Compare Memory and X Register
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CPX Immediate: E0(2)
     *   CPX Zero Page: E4(3)
     *   CPX Absolute: EC(4)
     */
    CompareAddressWithRegister( address, x );
  }

  void CPY( u16 address )
  {
    /* @brief Compare Memory and Y Register
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CPY Immediate: C0(2)
     *   CPY Zero Page: C4(3)
     *   CPY Absolute: CC(4)
     */
    CompareAddressWithRegister( address, y );
  }

  void PHA( const u16 address )
  {
    /* @brief Push Accumulator on Stack
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   PHA: 48(3)
     */
    (void) address;

    // Get the stack pointer
    const u8 stackPointer = GetStackPointer();

    // Push the accumulator onto the stack
    WriteByte( 0x0100 + stackPointer, GetAccumulator() );

    // Decrement the stack pointer
    SetStackPointer( stackPointer - 1 );
  }

  void PHP( const u16 address )
  {
    /* @brief Push Processor Status on Stack
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   PHP: 08(3)
     */
    (void) address;

    // Get the stack pointer
    const u8 stackPointer = GetStackPointer();

    // Set the Break flag before pushing the status register onto the stack
    u8 status = GetStatusRegister();
    status |= Break;

    // Push the modified status register onto the stack
    WriteByte( 0x0100 + stackPointer, status );

    SetStackPointer( stackPointer - 1 );
  }

  void PLA( const u16 address )
  {
    /* @brief Pop Accumulator from Stack
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   PLA: 68(4)
     */
    (void) address;

    // Increment the stack pointer first
    SetStackPointer( GetStackPointer() + 1 );

    // Get the accumulator from the stack and set the zero and negative flags
    SetAccumulator( ReadByte( 0x100 + GetStackPointer() ) );
    Tick(); // Dummy read
    SetZeroAndNegativeFlags( a );
  }

  void PLP( const u16 address )
  {
    /* @brief Pop Processor Status from Stack
     * N Z C I D V
     * from stack
     *   Usage and cycles:
     *   PLP: 28(4)
     */
    (void) address;

    // Increment the stack pointer first
    SetStackPointer( GetStackPointer() + 1 );

    SetStatusRegister( ReadByte( 0x100 + GetStackPointer() ) );
    ClearFlags( Status::Break );
    Tick(); // Dummy read
    SetFlags( Status::Unused );
  }

  void TSX( const u16 address )
  {
    /* @brief Transfer Stack Pointer to X
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TSX: BA(2)
     */
    (void) address;
    SetXRegister( GetStackPointer() );
    SetZeroAndNegativeFlags( GetXRegister() );
  }

  void TXS( const u16 address )
  {
    /* @brief Transfer X to Stack Pointer
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   TXS: 9A(2)
     */
    (void) address;
    SetStackPointer( GetXRegister() );
  }

  void ASL( u16 address )
  {
    /* @brief Arithmetic Shift Left
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ASL Accumulator: 0A(2)
     *   ASL Zero Page: 06(5)
     *   ASL Zero Page X: 16(6)
     *   ASL Absolute: 0E(6)
     *   ASL Absolute X: 1E(7)
     */

    if ( addrMode == "IMP" ) {
      u8 accumulator = GetAccumulator();
      // Set the carry flag if bit 7 is set
      ( accumulator & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      // Shift the accumulator left by one
      accumulator <<= 1;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( accumulator );

      // Set the new accumulator value
      SetAccumulator( accumulator );
    } else {
      u8 const value = ReadByte( address );

      Tick(); // simulate dummy write

      // Set the carry flag if bit 7 is set
      ( value & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      u8 const result = value << 1;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( result );

      // Write the result back to memory
      WriteByte( address, result );
    }
  }

  void LSR( u16 address )
  {
    /* @brief Logical Shift Right
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   LSR Accumulator: 4A(2)
     *   LSR Zero Page: 46(5)
     *   LSR Zero Page X: 56(6)
     *   LSR Absolute: 4E(6)
     *   LSR Absolute X: 5E(7)
     */

    if ( addrMode == "IMP" ) {
      u8 accumulator = GetAccumulator();
      // Set the carry flag if bit 0 is set
      ( accumulator & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      // Shift the accumulator right by one
      accumulator >>= 1;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( accumulator );

      // Set the new accumulator value
      SetAccumulator( accumulator );
    } else {
      u8 const value = ReadByte( address );
      Tick(); // simulate dummy write

      // Set the carry flag if bit 0 is set
      ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      u8 const result = value >> 1;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( result );

      // Write the result back to memory
      WriteByte( address, result );
    }
  }

  void ROL( u16 address )
  {
    /* @brief Rotate Left
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ROL Accumulator: 2A(2)
     *   ROL Zero Page: 26(5)
     *   ROL Zero Page X: 36(6)
     *   ROL Absolute: 2E(6)
     *   ROL Absolute X: 3E(7)
     */

    const u8 carry = IsFlagSet( Status::Carry ) ? 1 : 0;
    if ( addrMode == "IMP" ) {
      u8 accumulator = GetAccumulator();

      // Set the carry flag if bit 7 is set
      ( accumulator & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      // Shift the accumulator left by one
      accumulator <<= 1;

      // Add the carry to the accumulator
      accumulator |= carry;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( accumulator );

      // Set the new accumulator value
      SetAccumulator( accumulator );
    } else {
      u8 const value = ReadByte( address );
      Tick(); // dummy write

      // Set the carry flag if bit 7 is set
      ( value & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      u8 result = value << 1;
      result |= carry;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( result );

      // Write the result back to memory
      WriteByte( address, result );
    }
  }

  void ROR( u16 address )
  {
    /* @brief Rotate Right
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ROR Accumulator: 6A(2)
     *   ROR Zero Page: 66(5)
     *   ROR Zero Page X: 76(6)
     *   ROR Absolute: 6E(6)
     *   ROR Absolute X: 7E(7)
     */

    const u8 carry = IsFlagSet( Status::Carry ) ? 1 : 0;

    if ( addrMode == "IMP" ) { // implied mode
      u8 accumulator = GetAccumulator();

      // Set the carry flag if bit 0 is set
      ( accumulator & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      // Shift the accumulator right by one
      accumulator >>= 1;

      // Add the carry to the accumulator
      accumulator |= carry << 7;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( accumulator );

      // Set the new accumulator value
      SetAccumulator( accumulator );
    } else { // Memory mode
      u8 const value = ReadByte( address );
      Tick(); // simulate dummy write

      // Set the carry flag if bit 0 is set
      ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

      u8 result = value >> 1;
      result |= carry << 7;

      // Set the zero and negative flags
      SetZeroAndNegativeFlags( result );

      // Write the result back to memory
      WriteByte( address, result );
    }
  }

  void JMP( u16 address )
  {
    /* @brief Jump to New Location
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   JMP Absolute: 4C(3)
     *   JMP Indirect: 6C(5)
     */
    pc = address;
  }

  void JSR( u16 address )
  {
    /* @brief Jump to Sub Routine, Saving Return Address
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   JSR Absolute: 20(6)
     */
    u16 const returnAddress = pc - 1;
    Tick(); // Additional read here, probably for timing purposes
    StackPush( ( returnAddress >> 8 ) & 0xFF );
    StackPush( returnAddress & 0xFF );
    pc = address;
  }

  void RTS( const u16 address )
  {
    /* @brief Return from Subroutine
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   RTS: 60(6)
     */
    (void) address;
    u16 const low = StackPop();
    u16 const high = StackPop();
    pc = ( high << 8 ) | low;
    Tick(); // Account for reading the new address
    pc++;
    Tick(); // Account for reading the next pc value
  }

  void RTI( const u16 address )
  {
    /* @brief Return from Interrupt
     * N Z C I D V
     * from stack
     *   Usage and cycles:
     *   RTI: 40(6)
     */
    (void) address;
    u8 const status = StackPop();

    // Ignore the break flag and ensure the unused flag (bit 5) is set
    p = ( status & ~Break ) | Unused;

    u16 const low = StackPop();
    u16 const high = StackPop();
    pc = ( high << 8 ) | low;
    Tick(); // Account for reading the new address
  }

  void BRK( const u16 address )
  {
    /* @brief Force Interrupt
     * N Z C I D V
     * from stack
     *   Usage and cycles:
     *   BRK: 00(7)
     * Cycles:
     *   Read opcode: 1, Read padding byte: 1
     *   Push PC(2): 2, Push status(1): 1
     *   Read vector low: 1, Read vector high: 1
     */
    (void) address;
    pc++; // padding byte

    // Push pc to the stack
    StackPush( pc >> 8 );     // 1 cycle
    StackPush( pc & 0x00FF ); // 1 cycle

    // Push status with break and unused flag set (ignored when popped)
    StackPush( p | Break | Unused );

    // Set PC to the value at the interrupt vector (0xFFFE)
    u16 const low = ReadByte( 0xFFFE );
    u16 const high = ReadByte( 0xFFFF );
    pc = ( high << 8 ) | low;

    // Set the interrupt disable flag
    SetFlags( InterruptDisable );
  }

  void AND( u16 address )
  {
    /* @brief XOR Memory with Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   AND Immediate: 29(2)
     *   AND Zero Page: 25(3)
     *   AND Zero Page X: 35(4)
     *   AND Absolute: 2D(4)
     *   AND Absolute X: 3D(4+)
     *   AND Absolute Y: 39(4+)
     *   AND Indirect X: 21(6)
     *   AND Indirect Y: 31(5+)
     */
    u8 const value = ReadByte( address );
    a &= value;
    SetZeroAndNegativeFlags( a );
  }

  void ORA( const u16 address )
  {
    /* @brief OR Memory with Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   ORA Immediate: 09(2)
     *   ORA Zero Page: 05(3)
     *   ORA Zero Page X: 15(4)
     *   ORA Absolute: 0D(4)
     *   ORA Absolute X: 1D(4+)
     *   ORA Absolute Y: 19(4+)
     *   ORA Indirect X: 01(6)
     *   ORA Indirect Y: 11(5+)
     */

    u8 const value = ReadByte( address ); // 1 cycle
    a |= value;
    SetZeroAndNegativeFlags( a );
  }

  void EOR( const u16 address )
  {
    /* @brief XOR Memory with Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   EOR Immediate: 49(2)
     *   EOR Zero Page: 45(3)
     *   EOR Zero Page X: 55(4)
     *   EOR Absolute: 4D(4)
     *   EOR Absolute X: 5D(4+)
     *   EOR Absolute Y: 59(4+)
     *   EOR Indirect X: 41(6)
     *   EOR Indirect Y: 51(5+)
     */
    u8 const value = ReadByte( address );
    a ^= value;
    SetZeroAndNegativeFlags( a );
  }

  void BIT( const u16 address )
  {
    /* @brief Test Bits in Memory with Accumulator
     * Performs AND between accumulator and memory, but does not store the result
     * N Z C I D V
     * + + - - - +
     *   Usage and cycles:
     *   BIT Zero Page: 24(3)
     *   BIT Absolute: 2C(4)
     */

    u8 const value = ReadByte( address );
    SetZeroAndNegativeFlags( a & value );

    // Set overflow flag to bit 6 of value
    ( value & 0b01000000 ) != 0 ? SetFlags( Status::Overflow ) : ClearFlags( Status::Overflow );

    // Set negative flag to bit 7 of value
    ( value & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );
  }

  void TAX( const u16 address )
  {
    /* @brief Transfer Accumulator to X Register
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TAX: AA(2)
     */
    (void) address;
    SetXRegister( GetAccumulator() );
    SetZeroAndNegativeFlags( GetXRegister() );
  }

  void TXA( const u16 address )
  {
    /* @brief Transfer X Register to Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TXA: 8A(2)
     */
    (void) address;
    SetAccumulator( GetXRegister() );
    SetZeroAndNegativeFlags( GetAccumulator() );
  }

  void TAY( const u16 address )
  {
    /* @brief Transfer Accumulator to Y Register
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TAY: A8(2)
     */
    (void) address;
    SetYRegister( GetAccumulator() );
    SetZeroAndNegativeFlags( GetYRegister() );
  }

  void TYA( const u16 address )
  {
    /* @brief Transfer Y Register to Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TYA: 98(2)
     */
    (void) address;
    SetAccumulator( GetYRegister() );
    SetZeroAndNegativeFlags( GetAccumulator() );
  }

  /*
  ################################################################
  ||                                                            ||
  ||                      Illegal Opcodes                       ||
  ||                                                            ||
  ################################################################
  */

  void NOP2( u16 address ) // NOLINT
  {
    /*
     * @brief No operation, has an additional cycle
     * N Z C I D V
     * - - - - - -
     * --  Illegal  --
     * NOP Immediate: 80(2)
     * NOP Immediate: 82(2)
     * NOP Immediate: 89(2)
     * NOP Immediate: C2(2)
     * NOP Immediate: E2(2)
     * NOP Zero Page: 04(3)
     * NOP Zero Page: 44(3)
     * NOP Zero Page: 64(3)
     * NOP Zero Page X: 14(4)
     * NOP Zero Page X: 34(4)
     * NOP Zero Page X: 54(4)
     * NOP Zero Page X: 74(4)
     * NOP Zero Page X: D4(4)
     * NOP Zero Page X: F4(4)
     * NOP Absolute: 0C(4)
     * NOP Absolute: 1C(4)
     * NOP Absolute: 3C(4)
     * NOP Absolute: 5C(4)
     * NOP Absolute: 7C(4)
     * NOP Absolute: DC(4)
     * NOP Absolute: FC(4)
     */
    Tick();
    (void) address;
  }
  void JAM( const u16 address ) // NOLINT
  {
    /* @brief Illegal Opcode
     * Freezes the hardware, usually never called
     * Tom Harte tests include these, though, so for completeness, we'll add them
     */
    (void) address;
    for ( int i = 0; i < 9; i++ ) {
      Tick();
    }
  }

  void SLO( const u16 address )
  {
    /* @brief Illegal opcode: combines ASL and ORA
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   SLO Zero Page: 07(5)
     *   SLO Zero Page X: 17(6)
     *   SLO Absolute: 0F(6)
     *   SLO Absolute X: 1F(7)
     *   SLO Absolute Y: 1B(7)
     *   SLO Indirect X: 03(8)
     *   SLO Indirect Y: 13(8)
     */
    ASL( address );

    // ORA is side effect, no cycles are spent
    u8 const value = Read( address ); // 0 cycle
    a |= value;
    SetZeroAndNegativeFlags( a );
  }

  void SAX( const u16 address ) // NOLINT
  {
    /* @brief Illegal opcode: combines STX and AND
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   SAX Zero Page: 87(3)
     *   SAX Zero Page Y: 97(4)
     *   SAX Indirect X: 83(6)
     *   SAX Absolute: 8F(4)
     */
    WriteByte( address, a & x );
  }

  void LXA( const u16 address )
  {
    /* @brief Illegal opcode: combines LDA and LDX
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   ATX Immediate: AB(2)
     */

    u8 const magicConstant = 0xEE;
    u8 const value = ReadByte( address );

    u8 const result = ( ( a | magicConstant ) & value );
    a = result;
    x = result;
    SetZeroAndNegativeFlags( a );
  }

  void ATX( const u16 address )
  {
    // LDA & TAX
    u8 const value = ReadByte( address );
    x = value;
    a = value;
    SetZeroAndNegativeFlags( a );
  }

  void LAX( const u16 address )
  {
    /* @brief Illegal opcode: combines LDA and LDX
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   LAX Zero Page: A7(3)
     *   LAX Zero Page Y: B7(4)
     *   LAX Absolute: AF(4)
     *   LAX Absolute Y: BF(4+)
     *   LAX Indirect X: A3(6)
     *   LAX Indirect Y: B3(5+)
     */
    u8 const value = ReadByte( address );
    SetAccumulator( value );
    SetXRegister( value );
    SetZeroAndNegativeFlags( value );
  }

  void ARR( const u16 address )
  {
    /* @brief Illegal opcode: combines AND and ROR
     * N Z C I D V
     * + + + - - +
     *   Usage and cycles:
     *   ARR Immediate: 6B(2)
     */

    // A & operand
    u8 value = a & ReadByte( address );

    // ROR
    u8 const carryIn = IsFlagSet( Status::Carry ) ? 0x80 : 0x00;
    value = ( value >> 1 ) | carryIn;

    a = value;

    // Set flags
    SetZeroAndNegativeFlags( a );

    // Adjust C and V flags according to the ARR rules
    // C = bit 6 of A
    ( a & 0x40 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

    // V = bit 5 XOR bit 6
    bool const isOverflow = ( ( a & 0x40 ) != 0 ) ^ ( ( a & 0x20 ) != 0 );
    ( isOverflow ) ? SetFlags( Status::Overflow ) : ClearFlags( Status::Overflow );
  }

  void ALR( const u16 address )
  {
    /* @brief Illegal opcode: combines AND and LSR
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ALR Immediate: 4B(2)
     */
    AND( address );

    u8 const value = GetAccumulator();
    ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
    u8 const result = value >> 1;
    SetZeroAndNegativeFlags( result );
    a = result;
  }

  void RRA( const u16 address )
  {
    /* @brief Illegal opcode: combines ROR and ADC
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   RRA Zero Page: 67(5)
     *   RRA Zero Page X: 77(6)
     *   RRA Absolute: 6F(6)
     *   RRA Absolute X: 7F(7)
     *   RRA Absolute Y: 7B(7)
     *   RRA Indirect X: 63(8)
     *   RRA Indirect Y: 73(8)
     */
    ROR( address );
    ADC( address );
  }

  void SRE( const u16 address )
  {
    /* @brief Illegal opcode: combines LSR and EOR
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   SRE Zero Page: 47(5)
     *   SRE Zero Page X: 57(6)
     *   SRE Absolute: 4F(6)
     *   SRE Absolute X: 5F(7)
     *   SRE Absolute Y: 5B(7)
     *   SRE Indirect X: 43(8)
     *   SRE Indirect Y: 53(8)
     */
    LSR( address );

    // Free side effect
    u8 const value = Read( address ); // 0 cycle
    a ^= value;
    SetZeroAndNegativeFlags( a );
  }

  void RLA( const u16 address )
  {
    /* @brief Illegal opcode: combines ROL and AND
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   RLA Zero Page: 27(5)
     *   RLA Zero Page X: 37(6)
     *   RLA Absolute: 2F(6)
     *   RLA Absolute X: 3F(7)
     *   RLA Absolute Y: 3B(7)
     *   RLA Indirect X: 23(8)
     *   RLA Indirect Y: 33(8)
     */
    ROL( address );

    // Free side effect
    u8 const value = Read( address ); // 0 cycle
    a &= value;
    SetZeroAndNegativeFlags( a );
  }

  void DCP( const u16 address )
  {
    /* @brief Illegal opcode: combines DEC and CMP
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   DCP Zero Page: C7(5)
     *   DCP Zero Page X: D7(6)
     *   DCP Absolute: CF(6)
     *   DCP Absolute X: DF(7)
     *   DCP Absolute Y: DB(7)
     *   DCP Indirect X: C3(8)
     *   DCP Indirect Y: D3(8)
     */
    DEC( address );
    CMP( address );
  }

  void ISC( const u16 address )
  {
    /* @brief Illegal opcode: combines INC and SBC
     * N Z C I D V
     * + + + - - +
     *   Usage and cycles:
     *   ISC Zero Page: E7(5)
     *   ISC Zero Page X: F7(6)
     *   ISC Absolute: EF(6)
     *   ISC Absolute X: FF(7)
     *   ISC Absolute Y: FB(7)
     *   ISC Indirect X: E3(8)
     *   ISC Indirect Y: F3(8)
     */
    INC( address );
    SBC( address );
  }

  void ANC( const u16 address )
  {
    /* @brief Illegal opcode: combines AND and Carry
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ANC Immediate: 0B(2)
     *   ANC Immediate: 2B(2)
     */
    AND( address );
    ( IsFlagSet( Status::Negative ) ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
  }

  void SBX( const u16 address )
  {
    /* @brief Illegal opcode: SBX (a.k.a. AXS) combines CMP and DEX
     *        (A & X) - immediate -> X
     * Sets flags like CMP:
     *   N Z C I D V
     *   + + + - - -
     *
     * Usage and cycles:
     *   SBX Immediate: CB (2 bytes, 2 cycles)
     */
    u8 const  operand = ReadByte( address );
    u8 const  left = ( a & x );
    u16 const diff = static_cast<u16>( left ) - static_cast<u16>( operand );
    x = static_cast<u8>( diff & 0xFF );
    ( ( diff & 0x100 ) == 0 ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
    SetZeroAndNegativeFlags( x );
  }

  void LAS( const u16 address )
  {
    /* @brief Illegal opcode: LAS(LAR), combines LDA/TSX
     * M AND SP -> A, X, SP
     *
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   LAS Absolute Y: BB(4+)
     */
    u8 const memVal = ReadByte( address );
    u8 const sp = GetStackPointer();
    u8 const result = memVal & sp;

    a = result;
    x = result;
    s = result;

    SetZeroAndNegativeFlags( result );
  }

  void ANE( const u16 address )
  {
    /* @details Illegal opcode: ANE, combines AND and EOR
      * OR X + AND oper

      A base value in A is determined based on the contets of A and a constant, which may be typically
      $00, $ff, $ee, etc. The value of this constant depends on temerature, the chip series, and maybe
      other factors, as well. In order to eliminate these uncertaincies from the equation, use either 0 as
      the operand or a value of $FF in the accumulator.

      (A OR CONST) AND X AND oper -> A
      N	Z	C	I	D	V
      +	+	-	-	-	-
      addressing	assembler	opc	bytes	cycles
      immediate	ANE #oper	8B	2	2  	††

      Usage and cycles:
      * ANE Immediate: 8B(2)
    */
    u8 const operand = ReadByte( address );
    u8 const constant = 0xEE;

    // Compute: (A OR constant) AND X AND operand.
    u8 const result = ( a | constant ) & x & operand;
    a = result;

    SetZeroAndNegativeFlags( a );
  }

  void SyaSxaAxa( uint16_t baseAddr, uint8_t indexReg, uint8_t valueReg )
  {
    bool const pageCrossed = ( ( baseAddr & 0xFF00 ) != ( ( baseAddr + indexReg ) & 0xFF00 ) );
    auto       cyc = cycles;
    ReadByte( baseAddr + indexReg - ( pageCrossed ? 0x100 : 0 ) );
    bool const     hadDma = ( cycles - cyc ) > 1;
    uint16_t const operand = baseAddr + indexReg;
    uint8_t const  addrLow = uint8_t( operand & 0xFF );
    uint8_t        addrHigh = uint8_t( operand >> 8 );
    if ( pageCrossed ) {
      addrHigh &= valueReg;
    }
    uint8_t const toStore = hadDma ? valueReg : uint8_t( valueReg & uint8_t( ( baseAddr >> 8 ) + 1 ) );

    WriteByte( uint16_t( addrHigh ) << 8 | addrLow, toStore );
  }

  void SHY( u16 address )
  {
    u8 const  indexReg = x;
    u8 const  valueReg = y;
    u16 const baseAddr = address - indexReg;
    SyaSxaAxa( baseAddr, indexReg, valueReg );
  }

  void SHX( u16 address )
  {
    u8 const  indexReg = y;
    u8 const  valueReg = x;
    u16 const baseAddr = address - indexReg;
    SyaSxaAxa( baseAddr, indexReg, valueReg );
  }

  void SHA( const u16 address )
  {
    u8 const  valueReg = x & a;
    u8 const  indexReg = y;
    u16 const baseAddr = address - indexReg;
    SyaSxaAxa( baseAddr, indexReg, valueReg );
  }

  void TAS( const u16 address )
  {
    SHA( address );
    SetStackPointer( a & x );
  }

  void XXX( const u16 address ) // NOLINT
  {
    // unimplemented
    (void) address;
    fmt::print( "Unimplemented opcode: {:02X}\n", opcode );
  }
};

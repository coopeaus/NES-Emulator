// cpu_test.cpp
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "json.hpp"
#include <cstdint>
#include <fstream>
#include <gtest/gtest.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

// forward declarations
void printTestStartMsg( const std::string &testName );
void printTestEndMsg( const std::string &testName );
json extractTestsFromJson( const std::string &path );

class CPUTestFixture : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
  protected:
    // All tests assume flat memory model, which is why true is passed to Bus constructor
    Bus bus;
    CPU cpu;
    PPU ppu;

    CPUTestFixture() : cpu( bus.cpu ), ppu( bus.ppu ) {}

    void                      RunTestCase( const json &testCase );
    void                      LoadStateFromJson( const json &jsonData, const std::string &state );
    [[nodiscard]] std::string GetCPUStateString( const json &jsonData, const std::string &state ) const;

    // Expose private methods
    // CPUTestFixture is a friend class of CPU. To use cpu private methods, we need to create
    // wrappers.
    void               SetFlags( u8 flag );
    void               ClearFlags( u8 flag );
    [[nodiscard]] bool IsFlagSet( u8 flag ) const;
    [[nodiscard]] u8   Read( u16 address ) const;
    void               Write( u16 address, u8 data ) const;
    u16                IMM();
    u16                ZPG();
    u16                ZPGX();
    u16                ZPGY();
    u16                ABS();
    u16                ABSX();
    u16                ABSY();
    u16                IND();
    u16                INDX();
    u16                INDY();
    u16                REL();
};

/*
################################################################
||                                                            ||
||                     General Test Cases                     ||
||                                                            ||
################################################################
 */
TEST_F( CPUTestFixture, SanityCheck )
{
    // cpu.read and cpu.write shouldn't throw any errors
    u8 const test_val = Read( 0x0000 );
    Write( 0x0000, test_val );
}

TEST_F( CPUTestFixture, StatusFlags )
{
    constexpr u8 carry = 0b00000001;
    constexpr u8 zero = 0b00000010;
    constexpr u8 interrupt_disable = 0b00000100;
    constexpr u8 decimal = 0b00001000;
    constexpr u8 break_flag = 0b00010000;
    constexpr u8 unused = 0b00100000;
    constexpr u8 overflow = 0b01000000;
    constexpr u8 negative = 0b10000000;

    // Set and clear methods
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | unused );
    SetFlags( carry );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | unused );
    SetFlags( zero );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | zero | unused );
    SetFlags( interrupt_disable );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | zero | interrupt_disable | unused );
    SetFlags( decimal );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | zero | interrupt_disable | decimal | unused );
    SetFlags( break_flag );
    EXPECT_EQ( cpu.GetStatusRegister(),
               0x00 | carry | zero | interrupt_disable | decimal | break_flag | unused );
    ClearFlags( carry | zero | interrupt_disable | decimal | break_flag | unused );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 );
    SetFlags( overflow );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | overflow );
    SetFlags( negative );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | overflow | negative );
    // set all flags
    SetFlags( carry | zero | interrupt_disable | decimal | break_flag | overflow | negative | unused );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | zero | interrupt_disable | decimal | break_flag |
                                            overflow | negative | unused );
    // clear all flags
    ClearFlags( carry | zero | interrupt_disable | decimal | break_flag | overflow | negative | unused );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 );

    // IsFlagSet method
    EXPECT_FALSE( IsFlagSet( carry ) );
    SetFlags( carry );
    EXPECT_TRUE( IsFlagSet( carry ) );
    EXPECT_FALSE( IsFlagSet( zero ) );
    SetFlags( zero );
    EXPECT_TRUE( IsFlagSet( zero ) );
    EXPECT_TRUE( IsFlagSet( carry | zero ) );
}

/*
################################################################
||                                                            ||
||                   Addressing Mode Tests                    ||
||                                                            ||
################################################################
*/
TEST_F( CPUTestFixture, IMM )
{
    std::string const test_name = "Immediate Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x8000 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x8000 );
    u16 const addr = IMM();
    EXPECT_EQ( addr, 0x8000 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x8001 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, ZPG )
{
    std::string const test_name = "Zero Page Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    Write( 0x0000, 0x42 );
    u16 const addr = ZPG();
    EXPECT_EQ( addr, 0x42 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0001 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, ZPGX )
{
    std::string const test_name = "Zero Page X Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    cpu.SetXRegister( 0x1 );

    // Write zero-page address to the pc location
    Write( cpu.GetProgramCounter(), 0x80 );
    u8 const zero_page_address = Read( cpu.GetProgramCounter() );
    u8 const expected_address = ( zero_page_address + 0x1 ) & 0xFF;

    Write( expected_address, 0x42 ); // Write a test value at the expected address
    u16 const addr = ZPGX();
    EXPECT_EQ( addr, expected_address );
    EXPECT_EQ( Read( addr ), 0x42 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0001 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, ZPGY )
{
    std::string const test_name = "Zero Page Y Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    cpu.SetYRegister( 0x1 );
    // Write zero-page address to the pc location
    Write( cpu.GetProgramCounter(), 0x80 );
    u8 const zero_page_address = Read( cpu.GetProgramCounter() );
    u8 const expected_address = ( zero_page_address + 0x1 ) & 0xFF;
    Write( expected_address, 0x42 ); // Write a test value at the expected address
    u16 const addr = ZPGY();
    EXPECT_EQ( addr, expected_address );
    EXPECT_EQ( Read( addr ), 0x42 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0001 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, ABS )
{
    std::string const test_name = "Absolute Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    Write( 0x0000, 0x42 );
    Write( 0x0001, 0x24 );
    u16 const addr = ABS();
    EXPECT_EQ( addr, 0x2442 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0002 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, ABSX )
{
    std::string const test_name = "Absolute X Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    cpu.SetXRegister( 0x10 );
    // Write absolute address to the pc location
    Write( 0x0000, 0x80 );
    Write( 0x0001, 0x24 );

    // expected is absolute address + X register
    constexpr u16 expected_address = 0x2480 + 0x10;

    u16 const addr = ABSX();
    EXPECT_EQ( addr, expected_address );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0002 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, ABSY )
{
    std::string const test_name = "Absolute Y Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    cpu.SetYRegister( 0x10 );
    // Write absolute address to the pc location
    Write( 0x0000, 0x80 );
    Write( 0x0001, 0x24 );
    // expected is absolute address + Y register
    constexpr u16 expected_address = 0x2480 + 0x10;
    u16 const     addr = ABSY();
    EXPECT_EQ( addr, expected_address );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0002 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, IND )
{
    std::string const test_name = "Indirect Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );

    // Write the low and high byte of the pointer to the pc location
    Write( 0x0000, 0xCD );
    Write( 0x0001, 0xAB );

    // Write the effective address to the pointer location
    constexpr u16 ptr = 0xABCD;
    Write( ptr, 0x34 );
    Write( ptr + 1, 0x12 );

    // Get the effective address
    u16 const addr = IND();

    // Ensure the address is 0xABCD
    EXPECT_EQ( addr, 0x1234 ) << "Expected 0x1234, but got " << std::hex << addr;

    // Write a value at the effective address
    Write( addr, 0xEF );
    EXPECT_EQ( Read( addr ), 0xEF ) << "Expected 0xEF, but got " << static_cast<int>( Read( addr ) );

    // Ensure the pc is incremented by 2
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0002 );

    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, IND_Bug )
{
    std::string const test_name = "Indirect Addressing Mode Bug";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    Write( 0x0000, 0xFF );
    Write( 0x0001, 0x02 );
    constexpr u16 ptr = 0x02FF;

    // Write the low byte of the effective address to the pointer location
    Write( ptr, 0x34 );
    // Place where the high byte will be read from due to bug
    Write( 0x0200, 0x12 );
    // Place where the high byte will not be read from, but would have been without the bug
    Write( 0x0300, 0x56 );

    constexpr u16 bug_addr = 0x1234;
    Write( bug_addr, 0xEF );

    constexpr u16 no_bug_addr = 0x5634;
    Write( no_bug_addr, 0xAB );

    u16 const effective_addr = IND();
    EXPECT_EQ( effective_addr, bug_addr ) << "Expected 0x1234, but got " << std::hex << effective_addr;
    EXPECT_EQ( Read( effective_addr ), 0xEF )
        << "Expected 0xEF, but got " << static_cast<int>( Read( effective_addr ) );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0002 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, INDX )
{
    std::string const test_name = "Indirect X Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    cpu.SetXRegister( 0x10 );

    // Write the operand at the pc location
    constexpr u8 operand = 0x23;
    Write( 0x0000, operand );
    u8 const      ptr = ( operand + cpu.GetXRegister() ) & 0xFF;
    constexpr u16 effective_addr = 0xABCD;

    // Write the effective address to zero-page memory
    Write( ptr, effective_addr & 0xFF );
    Write( ( ptr + 1 ) & 0xFF, effective_addr >> 8 );

    // Write a test value at the effective address
    Write( effective_addr, 0x42 );

    u16 const addr = INDX();
    EXPECT_EQ( addr, effective_addr ) << "Expected " << std::hex << effective_addr << ", but got " << addr;
    EXPECT_EQ( Read( addr ), 0x42 ) << "Expected 0x42, but got " << int( Read( addr ) ); // NOLINT
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0001 );

    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, INDY )
{
    std::string const test_name = "Indirect Y Addressing Mode";
    printTestStartMsg( test_name );
    cpu.SetProgramCounter( 0x0000 );
    cpu.SetYRegister( 0x10 );

    constexpr u8 operand = 0x23;
    Write( 0x0000, operand );
    constexpr u8  ptr = operand;
    constexpr u16 effective_addr = 0xAB00;

    // Write the effective address to zero-page memory
    Write( ptr, effective_addr & 0xFF );
    Write( ptr + 1, effective_addr >> 8 );

    u16 const final_addr = effective_addr + cpu.GetYRegister();
    Write( final_addr, 0x42 ); // Write a test value at the final address
    u16 const addr = INDY();
    EXPECT_EQ( addr, final_addr ) << "Expected " << std::hex << final_addr << ", but got " << addr;
    EXPECT_EQ( Read( addr ), 0x42 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0001 );
    printTestEndMsg( test_name );
}

TEST_F( CPUTestFixture, REL )
{
    std::string const test_name = "Relative Addressing Mode";
    cpu.SetProgramCounter( 0x1000 );

    // Write a relative address of -5 at the pc location
    Write( 0x1000, 0xFB );
    u16 const back_branch = REL();
    EXPECT_EQ( back_branch, 0x0FFC ) << "Expected 0x0FFC, but got " << std::hex << back_branch;
    EXPECT_EQ( cpu.GetProgramCounter(), 0x1001 ) << "Expected PC to be 0x1001 after REL";

    // Write a relative address of +5 at the pc location
    cpu.SetProgramCounter( 0x1000 );
    Write( 0x1000, 0x05 );
    u16 const forward_branch = REL();
    EXPECT_EQ( forward_branch, 0x1006 ) << "Expected 0x1006, but got " << std::hex << forward_branch;
    EXPECT_EQ( cpu.GetProgramCounter(), 0x1001 ) << "Expected PC to be 0x1001 after REL";

    printTestStartMsg( test_name );
}

/*
################################################################
||                                                            ||
||                        Opcode Tests                        ||
||                                                            ||
################################################################
*/

/* This is a macro to simplify test creation for json tests
 * The naming convention is <opcode hex>_<mnemonic>_<addressing mode>
 * e.g. x00_BRK_Implied, x01_ORA_IndirectX, x05_ORA_ZeroPage, etc.
 */
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CPU_TEST( opcode_hex, mnemonic, addr_mode, filename )                                                \
    TEST_F( CPUTestFixture, x##opcode_hex##_##mnemonic##_##addr_mode )                                       \
    {                                                                                                        \
        std::string const testName = #opcode_hex " " #mnemonic " " #addr_mode;                               \
        printTestStartMsg( testName );                                                                       \
        json const testCases = extractTestsFromJson( "tests/json/" filename );                               \
        for ( const auto &testCase : testCases )                                                             \
        {                                                                                                    \
            RunTestCase( testCase );                                                                         \
        }                                                                                                    \
        printTestEndMsg( testName );                                                                         \
    }
// NOLINTEND(cppcoreguidelines-macro-usage)

/*
  Testing an opcode:
  1. The opcode is implemented in the CPU
  2. The JSON file exists in tests/json
  3. Uncomment the corresponding test below
  4. Build and run the tests

To isolate a test from the CLI:
  ./scripts/test.sh "CPUTestFixture.x00" # runs only the BRK test
  ./scripts/test.sh "CPUTestFixture.x01" # runs only the ORA IndirectX test
  ./scripts/test.sh "CPUTestFixture.x05" # runs only the ORA ZeroPage test

  or:
  ctest -R "CPUTestFixture.x00" # from the build directory

To run all tests:
  ./scripts/test.sh
  or
  ctest # from the build directory
*/

/* CPU_TEST( SAMPLE, JSON, SANITY_CHECK, "temp.json" ); */

/*
################################
||                            ||
||      Official Opcodes      ||
||                            ||
################################
*/
CPU_TEST( 00, BRK, Implied, "00.json" );
CPU_TEST( 01, ORA, IndirectX, "01.json" );
CPU_TEST( 05, ORA, ZeroPage, "05.json" );
CPU_TEST( 06, ASL, ZeroPage, "06.json" );
CPU_TEST( 08, PHP, Implied, "08.json" );
CPU_TEST( 09, ORA, Immediate, "09.json" );
CPU_TEST( 0A, ASL, Accumulator, "0a.json" );
CPU_TEST( 0D, ORA, Absolute, "0d.json" );
CPU_TEST( 0E, ASL, Absolute, "0e.json" );
CPU_TEST( 10, BPL, Relative, "10.json" );
CPU_TEST( 11, ORA, IndirectY, "11.json" );
CPU_TEST( 15, ORA, ZeroPageX, "15.json" );
CPU_TEST( 16, ASL, ZeroPageX, "16.json" );
CPU_TEST( 18, CLC, Implied, "18.json" );
CPU_TEST( 19, ORA, AbsoluteY, "19.json" );
CPU_TEST( 1D, ORA, AbsoluteX, "1d.json" );
CPU_TEST( 1E, ASL, AbsoluteX, "1e.json" );
CPU_TEST( 20, JSR, Absolute, "20.json" );
CPU_TEST( 21, AND, IndirectX, "21.json" );
CPU_TEST( 24, BIT, ZeroPage, "24.json" );
CPU_TEST( 25, AND, ZeroPage, "25.json" );
CPU_TEST( 26, ROL, ZeroPage, "26.json" );
CPU_TEST( 28, PLP, Implied, "28.json" );
CPU_TEST( 29, AND, Immediate, "29.json" );
CPU_TEST( 2A, ROL, Accumulator, "2a.json" );
CPU_TEST( 2C, BIT, Absolute, "2c.json" );
CPU_TEST( 2D, AND, Absolute, "2d.json" );
CPU_TEST( 2E, ROL, Absolute, "2e.json" );
CPU_TEST( 30, BMI, Relative, "30.json" );
CPU_TEST( 31, AND, IndirectY, "31.json" );
CPU_TEST( 35, AND, ZeroPageX, "35.json" );
CPU_TEST( 36, ROL, ZeroPageX, "36.json" );
CPU_TEST( 38, SEC, Implied, "38.json" );
CPU_TEST( 39, AND, AbsoluteY, "39.json" );
CPU_TEST( 3D, AND, AbsoluteX, "3d.json" );
CPU_TEST( 3E, ROL, AbsoluteX, "3e.json" );
CPU_TEST( 40, RTI, Implied, "40.json" );
CPU_TEST( 41, EOR, IndirectX, "41.json" );
CPU_TEST( 45, EOR, ZeroPage, "45.json" );
CPU_TEST( 46, LSR, ZeroPage, "46.json" );
CPU_TEST( 48, PHA, Implied, "48.json" );
CPU_TEST( 49, EOR, Immediate, "49.json" );
CPU_TEST( 4A, LSR, Accumulator, "4a.json" );
CPU_TEST( 4C, JMP, Absolute, "4c.json" );
CPU_TEST( 4D, EOR, Absolute, "4d.json" );
CPU_TEST( 4E, LSR, Absolute, "4e.json" );
CPU_TEST( 50, BVC, Relative, "50.json" );
CPU_TEST( 51, EOR, IndirectY, "51.json" );
CPU_TEST( 55, EOR, ZeroPageX, "55.json" );
CPU_TEST( 56, LSR, ZeroPageX, "56.json" );
CPU_TEST( 58, CLI, Implied, "58.json" );
CPU_TEST( 59, EOR, AbsoluteY, "59.json" );
CPU_TEST( 5D, EOR, AbsoluteX, "5d.json" );
CPU_TEST( 5E, LSR, AbsoluteX, "5e.json" );
CPU_TEST( 60, RTS, Implied, "60.json" );
CPU_TEST( 61, ADC, IndirectX, "61.json" );
CPU_TEST( 65, ADC, ZeroPage, "65.json" );
CPU_TEST( 66, ROR, ZeroPage, "66.json" );
CPU_TEST( 68, PLA, Implied, "68.json" );
CPU_TEST( 69, ADC, Immediate, "69.json" );
CPU_TEST( 6A, ROR, Accumulator, "6a.json" );
CPU_TEST( 6C, JMP, Indirect, "6c.json" );
CPU_TEST( 6D, ADC, Absolute, "6d.json" );
CPU_TEST( 6E, ROR, Absolute, "6e.json" );
CPU_TEST( 70, BVS, Relative, "70.json" );
CPU_TEST( 71, ADC, IndirectY, "71.json" );
CPU_TEST( 75, ADC, ZeroPageX, "75.json" );
CPU_TEST( 76, ROR, ZeroPageX, "76.json" );
CPU_TEST( 79, ADC, AbsoluteY, "79.json" );
CPU_TEST( 7D, ADC, AbsoluteX, "7d.json" );
CPU_TEST( 78, SEI, Implied, "78.json" );
CPU_TEST( 7E, ROR, AbsoluteX, "7e.json" );
CPU_TEST( 81, STA, IndirectX, "81.json" );
CPU_TEST( 84, STY, ZeroPage, "84.json" );
CPU_TEST( 85, STA, ZeroPage, "85.json" );
CPU_TEST( 86, STX, ZeroPage, "86.json" );
CPU_TEST( 88, DEY, Implied, "88.json" );
CPU_TEST( 8A, TXA, Implied, "8a.json" );
CPU_TEST( 8C, STY, Absolute, "8c.json" );
CPU_TEST( 8D, STA, Absolute, "8d.json" );
CPU_TEST( 8E, STX, Absolute, "8e.json" );
CPU_TEST( 90, BCC, Relative, "90.json" );
CPU_TEST( 91, STA, IndirectY, "91.json" );
CPU_TEST( 94, STY, ZeroPageX, "94.json" );
CPU_TEST( 95, STA, ZeroPageX, "95.json" );
CPU_TEST( 96, STX, ZeroPageY, "96.json" );
CPU_TEST( 98, TYA, Implied, "98.json" );
CPU_TEST( 99, STA, AbsoluteY, "99.json" );
CPU_TEST( 9A, TXS, Implied, "9a.json" );
CPU_TEST( 9D, STA, AbsoluteX, "9d.json" );
CPU_TEST( A1, LDA, IndirectX, "a1.json" );
CPU_TEST( A0, LDY, Immediate, "a0.json" );
CPU_TEST( A2, LDX, Immediate, "a2.json" );
CPU_TEST( A4, LDY, ZeroPage, "a4.json" );
CPU_TEST( A5, LDA, ZeroPage, "a5.json" );
CPU_TEST( A6, LDX, ZeroPage, "a6.json" );
CPU_TEST( A8, TAY, Implied, "a8.json" );
CPU_TEST( A9, LDA, Immediate, "a9.json" );
CPU_TEST( AA, TAX, Implied, "aa.json" );
CPU_TEST( AC, LDY, Absolute, "ac.json" );
CPU_TEST( AD, LDA, Absolute, "ad.json" );
CPU_TEST( AE, LDX, Absolute, "ae.json" );
CPU_TEST( B0, BCS, Relative, "b0.json" );
CPU_TEST( B1, LDA, IndirectY, "b1.json" );
CPU_TEST( B4, LDY, ZeroPageX, "b4.json" );
CPU_TEST( B5, LDA, ZeroPageX, "b5.json" );
CPU_TEST( B6, LDX, ZeroPageY, "b6.json" );
CPU_TEST( B8, CLV, Implied, "b8.json" );
CPU_TEST( B9, LDA, AbsoluteY, "b9.json" );
CPU_TEST( BA, TSX, Implied, "ba.json" );
CPU_TEST( BC, LDY, AbsoluteX, "bc.json" );
CPU_TEST( BD, LDA, AbsoluteX, "bd.json" );
CPU_TEST( BE, LDX, AbsoluteY, "be.json" );
CPU_TEST( C0, CPY, Immediate, "c0.json" );
CPU_TEST( C1, CMP, IndirectX, "c1.json" );
CPU_TEST( C4, CPY, ZeroPage, "c4.json" );
CPU_TEST( C5, CMP, ZeroPage, "c5.json" );
CPU_TEST( C6, DEC, ZeroPage, "c6.json" );
CPU_TEST( C8, INY, Implied, "c8.json" );
CPU_TEST( C9, CMP, Immediate, "c9.json" );
CPU_TEST( CA, DEX, Implied, "ca.json" );
CPU_TEST( CC, CPY, Absolute, "cc.json" );
CPU_TEST( CD, CMP, Absolute, "cd.json" );
CPU_TEST( CE, DEC, Absolute, "ce.json" );
CPU_TEST( D0, BNE, Relative, "d0.json" );
CPU_TEST( D1, CMP, IndirectY, "d1.json" );
CPU_TEST( D5, CMP, ZeroPageX, "d5.json" );
CPU_TEST( D6, DEC, ZeroPageX, "d6.json" );
CPU_TEST( D8, CLD, Implied, "d8.json" );
CPU_TEST( D9, CMP, AbsoluteY, "d9.json" );
CPU_TEST( DD, CMP, AbsoluteX, "dd.json" );
CPU_TEST( DE, DEC, AbsoluteX, "de.json" );
CPU_TEST( E0, CPX, Immediate, "e0.json" );
CPU_TEST( E1, SBC, IndirectX, "e1.json" );
CPU_TEST( E4, CPX, ZeroPage, "e4.json" );
CPU_TEST( E5, SBC, ZeroPage, "e5.json" );
CPU_TEST( E6, INC, ZeroPage, "e6.json" );
CPU_TEST( E8, INX, Implied, "e8.json" );
CPU_TEST( E9, SBC, Immediate, "e9.json" );
CPU_TEST( EA, NOP, Implied, "ea.json" );
CPU_TEST( EC, CPX, Absolute, "ec.json" );
CPU_TEST( ED, SBC, Absolute, "ed.json" );
CPU_TEST( EE, INC, Absolute, "ee.json" );
CPU_TEST( F0, BEQ, Relative, "f0.json" );
CPU_TEST( F1, SBC, IndirectY, "f1.json" );
CPU_TEST( F5, SBC, ZeroPageX, "f5.json" );
CPU_TEST( F6, INC, ZeroPageX, "f6.json" );
CPU_TEST( F9, SBC, AbsoluteY, "f9.json" );
CPU_TEST( FD, SBC, AbsoluteX, "fd.json" );
CPU_TEST( FE, INC, AbsoluteX, "fe.json" );
CPU_TEST( F8, SED, Implied, "f8.json" );

/*
################################
||       Illegal Opcodes      ||
################################
*/
CPU_TEST( 02, JAM, Implied, "02.json" );
CPU_TEST( 12, JAM, Implied, "12.json" );
CPU_TEST( 22, JAM, Implied, "22.json" );
CPU_TEST( 32, JAM, Implied, "32.json" );
CPU_TEST( 42, JAM, Implied, "42.json" );
CPU_TEST( 52, JAM, Implied, "52.json" );
CPU_TEST( 62, JAM, Implied, "62.json" );
CPU_TEST( 72, JAM, Implied, "72.json" );
CPU_TEST( 92, JAM, Implied, "92.json" );
CPU_TEST( B2, JAM, Implied, "b2.json" );
CPU_TEST( D2, JAM, Implied, "d2.json" );
CPU_TEST( F2, JAM, Implied, "f2.json" );
CPU_TEST( 1A, NOP, Implied, "1a.json" );
CPU_TEST( 3A, NOP, Implied, "3a.json" );
CPU_TEST( 5A, NOP, Implied, "5a.json" );
CPU_TEST( 7A, NOP, Implied, "7a.json" );
CPU_TEST( DA, NOP, Implied, "da.json" );
CPU_TEST( FA, NOP, Implied, "fa.json" );
CPU_TEST( 80, NOP, Immediate, "80.json" );
CPU_TEST( 82, NOP, Immediate, "82.json" );
CPU_TEST( 89, NOP, Immediate, "89.json" );
CPU_TEST( C2, NOP, Immediate, "c2.json" );
CPU_TEST( E2, NOP, Immediate, "e2.json" );
CPU_TEST( 04, NOP, ZeroPage, "04.json" );
CPU_TEST( 44, NOP, ZeroPage, "44.json" );
CPU_TEST( 64, NOP, ZeroPage, "64.json" );
CPU_TEST( 14, NOP, ZeroPageX, "14.json" );
CPU_TEST( 34, NOP, ZeroPageX, "34.json" );
CPU_TEST( 54, NOP, ZeroPageX, "54.json" );
CPU_TEST( 74, NOP, ZeroPageX, "74.json" );
CPU_TEST( D4, NOP, ZeroPageX, "d4.json" );
CPU_TEST( F4, NOP, ZeroPageX, "f4.json" );
CPU_TEST( 0C, NOP, Absolute, "0c.json" );
CPU_TEST( 1C, NOP, AbsoluteX, "1c.json" );
CPU_TEST( 3C, NOP, AbsoluteX, "3c.json" );
CPU_TEST( 5C, NOP, AbsoluteX, "5c.json" );
CPU_TEST( 7C, NOP, AbsoluteX, "7c.json" );
CPU_TEST( DC, NOP, AbsoluteX, "dc.json" );
CPU_TEST( FC, NOP, AbsoluteX, "fc.json" );
CPU_TEST( 07, SLO, ZeroPage, "07.json" );
CPU_TEST( 17, SLO, ZeroPageX, "17.json" );
CPU_TEST( 0F, SLO, Absolute, "0f.json" );
CPU_TEST( 1F, SLO, AbsoluteX, "1f.json" );
CPU_TEST( 1B, SLO, AbsoluteY, "1b.json" );
CPU_TEST( 03, SLO, IndirectX, "03.json" );
CPU_TEST( 13, SLO, IndirectY, "13.json" );
CPU_TEST( 27, RLA, ZeroPage, "27.json" );
CPU_TEST( 37, RLA, ZeroPageX, "37.json" );
CPU_TEST( 2F, RLA, Absolute, "2f.json" );
CPU_TEST( 3F, RLA, AbsoluteX, "3f.json" );
CPU_TEST( 3B, RLA, AbsoluteY, "3b.json" );
CPU_TEST( 23, RLA, IndirectX, "23.json" );
CPU_TEST( 33, RLA, IndirectY, "33.json" );
CPU_TEST( 47, SRE, ZeroPage, "47.json" );
CPU_TEST( 57, SRE, ZeroPageX, "57.json" );
CPU_TEST( 4F, SRE, Absolute, "4f.json" );
CPU_TEST( 5F, SRE, AbsoluteX, "5f.json" );
CPU_TEST( 5B, SRE, AbsoluteY, "5b.json" );
CPU_TEST( 43, SRE, IndirectX, "43.json" );
CPU_TEST( 53, SRE, IndirectY, "53.json" );
CPU_TEST( 67, RRA, ZeroPage, "67.json" );
CPU_TEST( 77, RRA, ZeroPageX, "77.json" );
CPU_TEST( 6F, RRA, Absolute, "6f.json" );
CPU_TEST( 7F, RRA, AbsoluteX, "7f.json" );
CPU_TEST( 7B, RRA, AbsoluteY, "7b.json" );
CPU_TEST( 63, RRA, IndirectX, "63.json" );
CPU_TEST( 73, RRA, IndirectY, "73.json" );
CPU_TEST( 87, SAX, ZeroPage, "87.json" );
CPU_TEST( 97, SAX, ZeroPageY, "97.json" );
CPU_TEST( 8F, SAX, Absolute, "8f.json" );
CPU_TEST( 83, SAX, IndirectX, "83.json" );
CPU_TEST( A7, LAX, ZeroPage, "a7.json" );
CPU_TEST( B7, LAX, ZeroPageY, "b7.json" );
CPU_TEST( AF, LAX, Absolute, "af.json" );
CPU_TEST( BF, LAX, AbsoluteY, "bf.json" );
CPU_TEST( A3, LAX, IndirectX, "a3.json" );
CPU_TEST( B3, LAX, IndirectY, "b3.json" );
CPU_TEST( C7, DCP, ZeroPage, "c7.json" );
CPU_TEST( D7, DCP, ZeroPageX, "d7.json" );
CPU_TEST( CF, DCP, Absolute, "cf.json" );
CPU_TEST( DF, DCP, AbsoluteX, "df.json" );
CPU_TEST( DB, DCP, AbsoluteY, "db.json" );
CPU_TEST( C3, DCP, IndirectX, "c3.json" );
CPU_TEST( D3, DCP, IndirectY, "d3.json" );
CPU_TEST( E7, ISC, ZeroPage, "e7.json" );
CPU_TEST( F7, ISC, ZeroPageX, "f7.json" );
CPU_TEST( EF, ISC, Absolute, "ef.json" );
CPU_TEST( FF, ISC, AbsoluteX, "ff.json" );
CPU_TEST( FB, ISC, AbsoluteY, "fb.json" );
CPU_TEST( E3, ISC, IndirectX, "e3.json" );
CPU_TEST( F3, ISC, IndirectY, "f3.json" );
CPU_TEST( 4B, ALR, Immediate, "4b.json" );
CPU_TEST( 6B, ARR, Immediate, "6b.json" );
CPU_TEST( EB, USBC, Immediate, "eb.json" );
CPU_TEST( 0B, ANC, Immediate, "0b.json" );
CPU_TEST( 2B, ANC, Immediate, "2b.json" );
CPU_TEST( AB, LXA, Immediate, "ab.json" );
CPU_TEST( CB, SBX, Immediate, "cb.json" );
/*
################################################################
||                                                            ||
||                    Test Fixture Methods                    ||
||                                                            ||
################################################################
*/

void CPUTestFixture::RunTestCase( const json &testCase ) // NOLINT
{
    // Initialize CPU
    bus.EnableJsonTestMode();
    cpu.EnableJsonTestMode();
    ppu.EnableJsonTestMode();
    cpu.Reset();

    LoadStateFromJson( testCase, "initial" );
    std::string const initial_state = GetCPUStateString( testCase, "initial" );
    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.GetProgramCounter(), static_cast<u16>( testCase["initial"]["pc"] ) );
    EXPECT_EQ( cpu.GetAccumulator(), testCase["initial"]["a"] );
    EXPECT_EQ( cpu.GetXRegister(), testCase["initial"]["x"] );
    EXPECT_EQ( cpu.GetYRegister(), testCase["initial"]["y"] );
    EXPECT_EQ( cpu.GetStackPointer(), testCase["initial"]["s"] );
    EXPECT_EQ( cpu.GetStatusRegister(), testCase["initial"]["p"] );

    for ( const auto &ram_entry : testCase["initial"]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  value = ram_entry[1];
        EXPECT_EQ( cpu.Read( address ), value );
    }

    // Fetch, decode, execute
    cpu.DecodeExecute();

    // Check final state
    bool               test_failed = false; // Track if any test has failed
    std::ostringstream error_messages;      // Accumulate error messages
                                            //
    if ( cpu.GetProgramCounter() != static_cast<u16>( testCase["final"]["pc"] ) )
    {
        test_failed = true;
        error_messages << "PC ";
    }
    if ( cpu.GetAccumulator() != static_cast<u8>( testCase["final"]["a"] ) )
    {
        test_failed = true;
        error_messages << "A ";
    }
    if ( cpu.GetXRegister() != static_cast<u8>( testCase["final"]["x"] ) )
    {
        test_failed = true;
        error_messages << "X ";
    }
    if ( cpu.GetYRegister() != static_cast<u8>( testCase["final"]["y"] ) )
    {
        test_failed = true;
        error_messages << "Y ";
    }
    if ( cpu.GetStackPointer() != static_cast<u8>( testCase["final"]["s"] ) )
    {
        test_failed = true;
        error_messages << "S ";
    }
    if ( cpu.GetStatusRegister() != static_cast<u8>( testCase["final"]["p"] ) )
    {
        test_failed = true;
        error_messages << "P ";
    }
    if ( cpu.GetCycles() != testCase["cycles"].size() )
    {
        test_failed = true;
        error_messages << "Cycle count ";
    }

    for ( const auto &ram_entry : testCase["final"]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  expected_value = ram_entry[1];
        uint8_t const  actual_value = cpu.Read( address );
        if ( actual_value != expected_value )
        {
            test_failed = true;
            error_messages << "RAM ";
        }
    }

    std::string const final_state = GetCPUStateString( testCase, "final" );
    // print initial and final state if there are any failures
    if ( test_failed )
    {
        std::cout << "Test Case: " << testCase["name"] << '\n';
        std::cout << "Failed: " << error_messages.str() << '\n';
        std::cout << initial_state << '\n';
        std::cout << final_state << '\n';
        std::cout << '\n';
        FAIL();
    }
}

void CPUTestFixture::LoadStateFromJson( const json &jsonData, const std::string &state )
{
    /*
     This function loads the CPU state from json data.
     args:
      jsonData: JSON data returned by extractTestsFromJson
      state: "initial" or "final"
    */
    cpu.SetProgramCounter( jsonData[state]["pc"] );
    cpu.SetAccumulator( jsonData[state]["a"] );
    cpu.SetXRegister( jsonData[state]["x"] );
    cpu.SetYRegister( jsonData[state]["y"] );
    cpu.SetStackPointer( jsonData[state]["s"] );
    cpu.SetStatusRegister( jsonData[state]["p"] );

    // Load memory state from JSON
    for ( const auto &ram_entry : jsonData[state]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  value = ram_entry[1];
        cpu.Write( address, value );
    }
}

std::string CPUTestFixture::GetCPUStateString( const json &jsonData, const std::string &state ) const
{
    /*
    This function provides formatted output for expected vs. actual CPU state values,
    based on provided json data and actual CPU state.
    Args:
      jsonData: JSON data returned by extractTestsFromJson
      state: "initial" or "final"
    */

    // Expected values
    u16 const expected_pc = static_cast<u16>( jsonData[state]["pc"] );
    u8 const  expected_a = jsonData[state]["a"];
    u8 const  expected_x = jsonData[state]["x"];
    u8 const  expected_y = jsonData[state]["y"];
    u8 const  expected_s = jsonData[state]["s"];
    u8 const  expected_p = jsonData[state]["p"];
    u64 const expected_cycles = jsonData["cycles"].size();

    // Actual values
    u16 const actual_pc = cpu.GetProgramCounter();
    u8 const  actual_a = cpu.GetAccumulator();
    u8 const  actual_x = cpu.GetXRegister();
    u8 const  actual_y = cpu.GetYRegister();
    u8 const  actual_s = cpu.GetStackPointer();
    u8 const  actual_p = cpu.GetStatusRegister();
    u64 const actual_cycles = cpu.GetCycles();

    // Column Widths
    constexpr int label_width = 8;
    constexpr int value_width = 14;

    // Use osstringstream to collect output
    std::ostringstream output;

    // Print header
    output << "----------" << state << " State----------" << '\n';
    output << std::left << std::setw( label_width ) << "" << std::setw( value_width ) << "EXPECTED"
           << std::setw( value_width ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto print_line = [&]( const std::string &label, const uint64_t expected, const uint64_t actual )
    {
        auto to_hex_decimal_string = []( const uint64_t value, const int width )
        {
            std::stringstream str_stream;
            str_stream << std::hex << std::uppercase << std::setw( width ) << std::setfill( '0' ) << value
                       << " (" << std::dec << value << ")";
            return str_stream.str();
        };

        int width; // NOLINT
        if ( expected > 0xFFFF || actual > 0xFFFF )
        {
            width = 8;
        }
        else if ( expected > 0xFF || actual > 0xFF )
        {
            width = 4;
        }
        else
        {
            width = 2;
        }

        output << std::left << std::setw( label_width ) << label;
        output << std::setw( value_width ) << to_hex_decimal_string( expected, width );
        output << std::setw( value_width ) << to_hex_decimal_string( actual, width ) << '\n';
    };

    // Print registers
    print_line( "pc:", expected_pc, actual_pc );
    print_line( "s:", expected_s, actual_s );
    print_line( "a:", expected_a, actual_a );
    print_line( "x:", expected_x, actual_x );
    print_line( "y:", expected_y, actual_y );
    print_line( "p:", expected_p, actual_p );

    if ( state == "final" )
    {
        output << std::left << std::setw( label_width ) << "cycles:";
        output << std::setw( value_width ) << expected_cycles;
        output << std::setw( value_width ) << actual_cycles << '\n';
    }

    // Blank line and RAM header
    output << '\n' << "RAM" << '\n';

    // Print RAM entries
    for ( const auto &ram_entry : jsonData[state]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  expected_value = ram_entry[1];
        uint8_t const  actual_value = cpu.Read( address );

        // Helper lambda to format values as "HEX (DECIMAL)"
        auto format_value = []( const uint8_t value )
        {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' )
                << static_cast<int>( value ) << " (" << std::dec << static_cast<int>( value ) << ")";
            return oss.str();
        };

        // Format address as hex only (no decimal for addresses)
        std::ostringstream address_stream;
        address_stream << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;

        // Print formatted output
        output << std::left << std::setw( label_width ) << address_stream.str();
        output << std::setw( value_width ) << format_value( expected_value );
        output << std::setw( value_width ) << format_value( actual_value ) << '\n';
    }

    output << "--------------------------------" << '\n';
    output << '\n';

    // Return the accumulated string
    return output.str();
}
/*
########################################
||    Expose private methods here     ||
########################################
*/

void CPUTestFixture::SetFlags( const u8 flag ) { cpu.SetFlags( flag ); }
void CPUTestFixture::ClearFlags( const u8 flag ) { cpu.ClearFlags( flag ); }
bool CPUTestFixture::IsFlagSet( const u8 flag ) const { return cpu.IsFlagSet( flag ); }
u8   CPUTestFixture::Read( const u16 address ) const { return cpu.Read( address ); }
void CPUTestFixture::Write( const u16 address, const u8 data ) const { cpu.Write( address, data ); }
u16  CPUTestFixture::IMM() { return cpu.IMM(); }
u16  CPUTestFixture::ZPG() { return cpu.ZPG(); }
u16  CPUTestFixture::ZPGX() { return cpu.ZPGX(); }
u16  CPUTestFixture::ZPGY() { return cpu.ZPGY(); }
u16  CPUTestFixture::ABS() { return cpu.ABS(); }
u16  CPUTestFixture::ABSX() { return cpu.ABSX(); }
u16  CPUTestFixture::ABSY() { return cpu.ABSY(); }
u16  CPUTestFixture::IND() { return cpu.IND(); }
u16  CPUTestFixture::INDX() { return cpu.INDX(); }
u16  CPUTestFixture::INDY() { return cpu.INDY(); }
u16  CPUTestFixture::REL() { return cpu.REL(); }

/*
################################################################
||                                                            ||
||                  General Helper Functions                  ||
||                                                            ||
################################################################
*/

auto extractTestsFromJson( const std::string &path ) -> json
// Extracts test cases from a JSON file and returns them as a JSON object, with the
// help of the nlohmann::json library.
{
    std::ifstream json_file( path );
    if ( !json_file.is_open() )
    {
        throw std::runtime_error( "Could not open test file: " + path );
    }
    json test_cases;
    json_file >> test_cases;

    if ( !test_cases.is_array() )
    {
        throw std::runtime_error( "Expected an array of test cases in JSON file" );
    }
    return test_cases;
}

void printTestStartMsg( const std::string &testName )
{
    std::cout << '\n';
    std::cout << "---------- " << testName << " Tests ---------" << '\n';
}
void printTestEndMsg( const std::string &testName )
{
    std::cout << "---------- " << testName << " Tests Complete ---------" << '\n';
    std::cout << '\n';
}

// -----------------------------------------------------------------------------
// -------------------------------- MAIN ---------------------------------------
// -----------------------------------------------------------------------------
auto main( int argc, char **argv ) -> int
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

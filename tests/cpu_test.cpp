// cpu_test.cpp
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"
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
    Bus  bus;
    CPU &cpu = bus.cpu;
    PPU &ppu = bus.ppu;

    CPUTestFixture() = default;

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

    void LoadTestCartridge()
    {
        std::string romFile = "tests/roms/palette.nes";
        bus.cartridge.LoadRom( romFile );
        bus.cpu.Reset();
    }
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
    u8 const testVal = Read( 0x0000 );
    Write( 0x0000, testVal );
}

TEST_F( CPUTestFixture, MemorySweep )
{
    // Write to every place in addressable memory, ensure emulator doesn't crash
    try {
        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            Write( i, 0x00 );
        }

        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            auto dummyVal = Read( i );
            (void) dummyVal;
        }
    } catch ( std::exception &e ) {
        FAIL() << e.what();
    }

    LoadTestCartridge();

    try {
        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            Write( i, 0x00 );
        }

        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            auto dummyVal = Read( i );
            (void) dummyVal;
        }
    } catch ( std::exception &e ) {
        FAIL() << e.what();
    }
}

TEST_F( CPUTestFixture, RamCheck )
{
    // Write 1 to every location in RAM
    for ( u16 i = 0x0000; i < 0x0800; ++i ) {
        Write( i, 0x01 );
    }
    for ( u16 i = 0x0000; i < 0x0800; ++i ) {
        EXPECT_EQ( Read( i ), 0x01 );
    }

    bus.DebugReset();
    // Write 1 to mirrored locations in RAM
    for ( u16 i = 0x0800; i < 0x2000; ++i ) {
        Write( i, 0x01 );
    }
    for ( u16 i = 0x0000; i < 0x0800; ++i ) {
        EXPECT_EQ( Read( i ), 0x01 );
    }
}

TEST_F( CPUTestFixture, ResetVector )
{
    bus.EnableJsonTestMode(); // enables flat memory
    cpu.Write( 0xFFFC, 0x34 );
    cpu.Write( 0xFFFD, 0x12 );

    // PC should be 0
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0000 );
    cpu.Reset();

    // PC should be 0x1234
    EXPECT_EQ( cpu.GetProgramCounter(), 0x1234 );
}

TEST_F( CPUTestFixture, IRQ )
{
    bus.EnableJsonTestMode();
    bus.DebugReset();

    // I flag should be set
    EXPECT_EQ( cpu.GetInterruptDisableFlag(), 1 );

    // No IRQ when I flag is set
    auto cycles = cpu.GetCycles();
    cpu.IRQ();
    EXPECT_EQ( cpu.GetCycles(), cycles );

    // IRQ should execute when I flag is cleared
    cpu.SetInterruptDisableFlag( false );
    cycles = cpu.GetCycles();
    cpu.IRQ();
    EXPECT_EQ( cpu.GetCycles(), cycles + 7 );

    // Interrupt vector at 0xFFFE should set PC
    bus.EnableJsonTestMode();
    bus.DebugReset();
    cpu.Write( 0xFFFE, 0x34 );
    cpu.Write( 0xFFFF, 0x12 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0000 );
    cpu.SetInterruptDisableFlag( false );
    cpu.IRQ();
    EXPECT_EQ( cpu.GetProgramCounter(), 0x1234 );
}

TEST_F( CPUTestFixture, NMI )
{
    bus.EnableJsonTestMode();
    bus.DebugReset();

    // NMI should trigger, even when I flag is set
    auto cycles = cpu.GetCycles();
    cpu.NMI();
    EXPECT_EQ( cpu.GetCycles(), cycles + 7 );

    // NMI vector at 0xFFFA should set PC
    bus.EnableJsonTestMode();
    bus.DebugReset();
    cpu.Write( 0xFFFA, 0x34 );
    cpu.Write( 0xFFFB, 0x12 );
    EXPECT_EQ( cpu.GetProgramCounter(), 0x0000 );
    cpu.NMI();
    EXPECT_EQ( cpu.GetProgramCounter(), 0x1234 );
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
        for ( const auto &testCase : testCases ) {                                                           \
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
CPU_TEST( BB, LAS, AbsoluteY, "bb.json" );
CPU_TEST( 8B, ANE, AbsoluteY, "8b.json" );
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
    std::string const initialState = GetCPUStateString( testCase, "initial" );
    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.GetProgramCounter(), static_cast<u16>( testCase["initial"]["pc"] ) );
    EXPECT_EQ( cpu.GetAccumulator(), testCase["initial"]["a"] );
    EXPECT_EQ( cpu.GetXRegister(), testCase["initial"]["x"] );
    EXPECT_EQ( cpu.GetYRegister(), testCase["initial"]["y"] );
    EXPECT_EQ( cpu.GetStackPointer(), testCase["initial"]["s"] );
    EXPECT_EQ( cpu.GetStatusRegister(), testCase["initial"]["p"] );

    for ( const auto &ramEntry : testCase["initial"]["ram"] ) {
        uint16_t const address = ramEntry[0];
        uint8_t const  value = ramEntry[1];
        EXPECT_EQ( cpu.Read( address ), value );
    }

    // Fetch, decode, execute
    cpu.DecodeExecute();

    // Check final state
    bool               testFailed = false; // Track if any test has failed
    std::ostringstream errorMessages;      // Accumulate error messages
                                           //
    if ( cpu.GetProgramCounter() != static_cast<u16>( testCase["final"]["pc"] ) ) {
        testFailed = true;
        errorMessages << "PC ";
    }
    if ( cpu.GetAccumulator() != static_cast<u8>( testCase["final"]["a"] ) ) {
        testFailed = true;
        errorMessages << "A ";
    }
    if ( cpu.GetXRegister() != static_cast<u8>( testCase["final"]["x"] ) ) {
        testFailed = true;
        errorMessages << "X ";
    }
    if ( cpu.GetYRegister() != static_cast<u8>( testCase["final"]["y"] ) ) {
        testFailed = true;
        errorMessages << "Y ";
    }
    if ( cpu.GetStackPointer() != static_cast<u8>( testCase["final"]["s"] ) ) {
        testFailed = true;
        errorMessages << "S ";
    }
    if ( cpu.GetStatusRegister() != static_cast<u8>( testCase["final"]["p"] ) ) {
        testFailed = true;
        errorMessages << "P ";
    }
    if ( cpu.GetCycles() != testCase["cycles"].size() ) {
        testFailed = true;
        errorMessages << "Cycle count ";
    }

    for ( const auto &ramEntry : testCase["final"]["ram"] ) {
        uint16_t const address = ramEntry[0];
        uint8_t const  expectedValue = ramEntry[1];
        uint8_t const  actualValue = cpu.Read( address );
        if ( actualValue != expectedValue ) {
            testFailed = true;
            errorMessages << "RAM ";
        }
    }

    std::string const finalState = GetCPUStateString( testCase, "final" );
    // print initial and final state if there are any failures
    if ( testFailed ) {
        std::cout << "Test Case: " << testCase["name"] << '\n';
        std::cout << "Failed: " << errorMessages.str() << '\n';
        std::cout << initialState << '\n';
        std::cout << finalState << '\n';
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
    for ( const auto &ramEntry : jsonData[state]["ram"] ) {
        uint16_t const address = ramEntry[0];
        uint8_t const  value = ramEntry[1];
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
    u16 const expectedPc = static_cast<u16>( jsonData[state]["pc"] );
    u8 const  expectedA = jsonData[state]["a"];
    u8 const  expectedX = jsonData[state]["x"];
    u8 const  expectedY = jsonData[state]["y"];
    u8 const  expectedS = jsonData[state]["s"];
    u8 const  expectedP = jsonData[state]["p"];
    u64 const expectedCycles = jsonData["cycles"].size();

    // Actual values
    u16 const actualPc = cpu.GetProgramCounter();
    u8 const  actualA = cpu.GetAccumulator();
    u8 const  actualX = cpu.GetXRegister();
    u8 const  actualY = cpu.GetYRegister();
    u8 const  actualS = cpu.GetStackPointer();
    u8 const  actualP = cpu.GetStatusRegister();
    u64 const actualCycles = cpu.GetCycles();

    // Column Widths
    constexpr int labelWidth = 8;
    constexpr int valueWidth = 14;

    // Use osstringstream to collect output
    std::ostringstream output;

    // Print header
    output << "----------" << state << " State----------" << '\n';
    output << std::left << std::setw( labelWidth ) << "" << std::setw( valueWidth ) << "EXPECTED"
           << std::setw( valueWidth ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto printLine = [&]( const std::string &label, const uint64_t expected, const uint64_t actual ) {
        auto toHexDecimalString = []( const uint64_t value, const int width ) {
            std::stringstream strStream;
            strStream << std::hex << std::uppercase << std::setw( width ) << std::setfill( '0' ) << value
                      << " (" << std::dec << value << ")";
            return strStream.str();
        };

        int width; // NOLINT
        if ( expected > 0xFFFF || actual > 0xFFFF ) {
            width = 8;
        } else if ( expected > 0xFF || actual > 0xFF ) {
            width = 4;
        } else {
            width = 2;
        }

        output << std::left << std::setw( labelWidth ) << label;
        output << std::setw( valueWidth ) << toHexDecimalString( expected, width );
        output << std::setw( valueWidth ) << toHexDecimalString( actual, width ) << '\n';
    };

    // Print registers
    printLine( "pc:", expectedPc, actualPc );
    printLine( "s:", expectedS, actualS );
    printLine( "a:", expectedA, actualA );
    printLine( "x:", expectedX, actualX );
    printLine( "y:", expectedY, actualY );
    printLine( "p:", expectedP, actualP );

    if ( state == "final" ) {
        output << std::left << std::setw( labelWidth ) << "cycles:";
        output << std::setw( valueWidth ) << expectedCycles;
        output << std::setw( valueWidth ) << actualCycles << '\n';
    }

    // Blank line and RAM header
    output << '\n' << "RAM" << '\n';

    // Print RAM entries
    for ( const auto &ramEntry : jsonData[state]["ram"] ) {
        uint16_t const address = ramEntry[0];
        uint8_t const  expectedValue = ramEntry[1];
        uint8_t const  actualValue = cpu.Read( address );

        // Helper lambda to format values as "HEX (DECIMAL)"
        auto formatValue = []( const uint8_t value ) {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' )
                << static_cast<int>( value ) << " (" << std::dec << static_cast<int>( value ) << ")";
            return oss.str();
        };

        // Format address as hex only (no decimal for addresses)
        std::ostringstream addressStream;
        addressStream << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;

        // Print formatted output
        output << std::left << std::setw( labelWidth ) << addressStream.str();
        output << std::setw( valueWidth ) << formatValue( expectedValue );
        output << std::setw( valueWidth ) << formatValue( actualValue ) << '\n';
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

void CPUTestFixture::SetFlags( const u8 flag )
{
    cpu.SetFlags( flag );
}
void CPUTestFixture::ClearFlags( const u8 flag )
{
    cpu.ClearFlags( flag );
}
bool CPUTestFixture::IsFlagSet( const u8 flag ) const
{
    return cpu.IsFlagSet( flag );
}
u8 CPUTestFixture::Read( const u16 address ) const
{
    return cpu.Read( address );
}
void CPUTestFixture::Write( const u16 address, const u8 data ) const
{
    cpu.Write( address, data );
}
u16 CPUTestFixture::IMM()
{
    return cpu.IMM();
}
u16 CPUTestFixture::ZPG()
{
    return cpu.ZPG();
}
u16 CPUTestFixture::ZPGX()
{
    return cpu.ZPGX();
}
u16 CPUTestFixture::ZPGY()
{
    return cpu.ZPGY();
}
u16 CPUTestFixture::ABS()
{
    return cpu.ABS();
}
u16 CPUTestFixture::ABSX()
{
    return cpu.ABSX();
}
u16 CPUTestFixture::ABSY()
{
    return cpu.ABSY();
}
u16 CPUTestFixture::IND()
{
    return cpu.IND();
}
u16 CPUTestFixture::INDX()
{
    return cpu.INDX();
}
u16 CPUTestFixture::INDY()
{
    return cpu.INDY();
}
u16 CPUTestFixture::REL()
{
    return cpu.REL();
}

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
    std::ifstream jsonFile( path );
    if ( !jsonFile.is_open() ) {
        throw std::runtime_error( "Could not open test file: " + path );
    }
    json testCases;
    jsonFile >> testCases;

    if ( !testCases.is_array() ) {
        throw std::runtime_error( "Expected an array of test cases in JSON file" );
    }
    return testCases;
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

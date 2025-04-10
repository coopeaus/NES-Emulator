#include "cpu_test.h"

/*
################################################################
||                                                            ||
||                     General Test Cases                     ||
||                                                            ||
################################################################
 */
TEST_F( CpuTest, SanityCheck )
{
    // cpu.read and cpu.write shouldn't throw any errors
    u8 const testVal = cpu.Read( 0x0000 );
    cpu.Write( 0x0000, testVal );
}

TEST_F( CpuTest, MemorySweep )
{
    // Write to every place in addressable memory, ensure emulator doesn't crash
    try {
        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            cpu.Write( i, 0x00 );
        }

        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            auto dummyVal = cpu.Read( i );
            (void) dummyVal;
        }
    } catch ( std::exception &e ) {
        FAIL() << e.what();
    }

    LoadTestCartridge();

    try {
        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            cpu.Write( i, 0x00 );
        }

        for ( u16 i = 0; i < 0xFFFF; ++i ) {
            auto dummyVal = cpu.Read( i );
            (void) dummyVal;
        }
    } catch ( std::exception &e ) {
        FAIL() << e.what();
    }
}

TEST_F( CpuTest, RamCheck )
{
    // Write 1 to every location in RAM
    for ( u16 i = 0x0000; i < 0x0800; ++i ) {
        cpu.Write( i, 0x01 );
    }
    for ( u16 i = 0x0000; i < 0x0800; ++i ) {
        EXPECT_EQ( cpu.Read( i ), 0x01 );
    }

    bus.DebugReset();
    // Write 1 to mirrored locations in RAM
    for ( u16 i = 0x0800; i < 0x2000; ++i ) {
        cpu.Write( i, 0x01 );
    }
    for ( u16 i = 0x0000; i < 0x0800; ++i ) {
        EXPECT_EQ( cpu.Read( i ), 0x01 );
    }
}

TEST_F( CpuTest, ResetVector )
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

TEST_F( CpuTest, IRQ )
{
    bus.EnableJsonTestMode();
    bus.DebugReset();

    EXPECT_EQ( cpu.GetInterruptDisableFlag(), 0 );
    cpu.SetInterruptDisableFlag( true );

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

TEST_F( CpuTest, NMI )
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

TEST_F( CpuTest, ExecuteFrame )
{
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
 * e.g. x00_BRK_Implied, x01_ORA_IndirectX, x05_ORA_ZeroPage, etc. */
#define CPU_TEST( opcode_hex, mnemonic, addr_mode, filename )                                                \
    TEST_F( CpuTest, x##opcode_hex##_##mnemonic##_##addr_mode )                                              \
    {                                                                                                        \
        std::string       space = " ";                                                                       \
        std::string const testName = #opcode_hex + space + #mnemonic + space + #addr_mode;                   \
        TestStart( testName );                                                                               \
        std::string jsonDir = std::string( paths::tests() ) + "/json/" + ( filename );                       \
        json const  testCases = GetJsonData( jsonDir );                                                      \
        for ( const auto &testCase : testCases ) {                                                           \
            RunTestCase( testCase );                                                                         \
        }                                                                                                    \
        TestEnd( testName );                                                                                 \
    }

/*
################################
||      Official Opcodes      ||
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
CPU_TEST( 8B, ANE, Immediate, "8b.json" );

auto main( int argc, char **argv ) -> int
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

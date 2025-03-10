// Google Test Basics: TEST and TEST_F
//
// - `TEST`: Defines a standalone test case.
// - `TEST_F`: Defines a test case that uses a fixture (a shared setup/teardown
// environment).
//
// Common Assertions:
// - EXPECT_EQ(val1, val2): Checks if val1 == val2 (non-fatal, continues test on
// failure).
// - ASSERT_EQ(val1, val2): Checks if val1 == val2 (fatal, stops test on
// failure).
// - EXPECT_NE, EXPECT_LT, EXPECT_GT, EXPECT_LE, EXPECT_GE: Comparison macros.
// - EXPECT_TRUE(condition) / EXPECT_FALSE(condition): Checks a boolean
// condition.
//
// Basic Usage:
// 1. Include <gtest/gtest.h>.
// 2. Define tests using `TEST` or `TEST_F`.
// 3. Compile using settings from CMakeLists.txt and run with `ctest`.
//
// Below is an example of both `TEST` and `TEST_F` usage.

#include "bus.h"
#include "cartridge.h"
#include <fmt/base.h>
#include <gtest/gtest.h>

class PpuTest : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
  protected:
    // All tests assume flat memory model, which is why true is passed to Bus constructor
    Bus  bus;
    PPU &ppu = bus.ppu;
    CPU &cpu = bus.cpu;

    PpuTest()
    {
        std::string romFile = "tests/roms/palette.nes";
        bus.cartridge.LoadRom( romFile );
        bus.cpu.Reset();
    }
};

TEST_F( PpuTest, Write2000 )
{
    cpu.Write( 0x2000, 0x80 );
    EXPECT_EQ( ppu.ppuCtrl.bit.nmiEnable, 1 );

    ppu.ppuCtrl.bit.nametableX = 1;
    ppu.ppuCtrl.bit.nametableY = 1;
    auto data = ppu.ppuCtrl.value;
    cpu.Write( 0x2000, data );
    EXPECT_EQ( ppu.tempAddr.bit.nametableX, 1 );
    EXPECT_EQ( ppu.tempAddr.bit.nametableY, 1 );
}

TEST_F( PpuTest, Write2001 )
{
    cpu.Write( 0x2001, 0x1 );
    EXPECT_EQ( ppu.GetMaskGrayscale(), 1 );
}

TEST_F( PpuTest, Write2005Scroll ) // Scroll
{
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    u8 data = 0x10;
    cpu.Write( 0x2005, data );
    EXPECT_EQ( ppu.GetAddrLatch(), 1 );
    EXPECT_EQ( ppu.GetTempCoarseX(), data >> 3 );
    cpu.Write( 0x2005, data );
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    EXPECT_EQ( ppu.GetTempCoarseY(), data >> 3 );
    EXPECT_EQ( ppu.GetFineX(), data & 0x7 );
}

TEST_F( PpuTest, Write2006Addr ) // PPU latch
{
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    u8 data1 = 0x12;
    u8 data2 = 0x34;
    cpu.Write( 0x2006, data1 );
    EXPECT_EQ( ppu.GetAddrLatch(), 1 );
    EXPECT_EQ( ppu.GetTempAddr(), data1 << 8 );
    cpu.Write( 0x2006, data2 );
    EXPECT_EQ( ppu.GetAddrLatch(), 0 );
    EXPECT_EQ( ppu.GetTempAddr(), data1 << 8 | data2 );
}

TEST_F( PpuTest, Write2007 )
{
    ppu.ppuCtrl.bit.vramIncrement = 0;
    cpu.Write( 0x2007, 0 );
    EXPECT_EQ( ppu.vramAddr.value, 0x1 );
    ppu.ppuCtrl.bit.vramIncrement = 1;
    cpu.Write( 0x2007, 0 );
    EXPECT_EQ( ppu.vramAddr.value, 0x1 + 32 );
}

TEST_F( PpuTest, Read2002 )
{
    // should clear vblank
    ppu.ppuStatus.bit.verticalBlank = 1;
    auto data = cpu.Read( 0x2002 );
    EXPECT_EQ( ppu.ppuStatus.bit.verticalBlank, 0 );
    EXPECT_EQ( data, 0x80 );
}

TEST_F( PpuTest, DmaTransfer )
{
    // fill some placeholder values in the cpu ram
    for ( u16 i = 0; i < 256; i++ ) {
        cpu.Write( 0x200 + i, i );
    }

    // now, hit the dma spot with 0x02
    cpu.Write( 0x4014, 0x02 );

    // Data should have transferred
    for ( u16 i = 0; i < 256; i++ ) {
        auto val = cpu.Read( 0x200 + i );
        EXPECT_EQ( ppu.oam[i], val );
    }
}

TEST_F( PpuTest, ResolveNameTableAddress_SingleUpper )
{
    u16 addr = 0x23AB;
    // Expected: 0x2000 OR (addr & 0x03FF)
    u16 expected = 0x2000 | ( addr & 0x03FF );
    u16 result = ppu.ResolveNameTableAddress( addr, static_cast<int>( MirrorMode::SingleUpper ) );
    EXPECT_EQ( result, expected );
}

TEST_F( PpuTest, ResolveNameTableAddress_SingleLower )
{
    u16 addr = 0x2434;
    u16 expected = 0x2800 | ( addr & 0x03FF );
    u16 result = ppu.ResolveNameTableAddress( addr, static_cast<int>( MirrorMode::SingleLower ) );
    EXPECT_EQ( result, expected );
}

TEST_F( PpuTest, ResolveNameTableAddress_Vertical )
{
    // Test an address in NT0 range.
    u16 addr1 = 0x2000;
    u16 expected1 = 0x2000 | ( addr1 & 0x07FF );
    u16 result1 = ppu.ResolveNameTableAddress( addr1, static_cast<int>( MirrorMode::Vertical ) );
    EXPECT_EQ( result1, expected1 );

    // Test an address in the mirrored portion.
    u16 addr2 = 0x2C00;
    // 0x2C00 & 0x07FF gives 0x0400, so expected address is 0x2000 + 0x0400 = 0x2400.
    u16 expected2 = 0x2000 | ( addr2 & 0x07FF );
    u16 result2 = ppu.ResolveNameTableAddress( addr2, static_cast<int>( MirrorMode::Vertical ) );
    EXPECT_EQ( result2, expected2 );
}

TEST_F( PpuTest, ResolveNameTableAddress_Horizontal )
{
    // Address in NT0 region (both 0x2000-0x23FF and its mirror 0x2400-0x27FF should resolve to NT0).
    u16 addr1 = 0x2400;                          // should map to 0x2000 + (addr1 mod 0x400)
    u16 expected1 = 0x2000 | ( addr1 & 0x03FF ); // 0x2400 & 0x03FF is 0, so becomes 0x2000.
    u16 result1 = ppu.ResolveNameTableAddress( addr1, static_cast<int>( MirrorMode::Horizontal ) );
    EXPECT_EQ( result1, expected1 );

    // Address in NT1 region (both 0x2800-0x2BFF and its mirror 0x2C00-0x2FFF should resolve to NT1).
    u16 addr2 = 0x2C00;                          // should map to 0x2800 + (addr2 mod 0x400)
    u16 expected2 = 0x2800 | ( addr2 & 0x03FF ); // For 0x2C00, (addr2 & 0x03FF) is 0.
    u16 result2 = ppu.ResolveNameTableAddress( addr2, static_cast<int>( MirrorMode::Horizontal ) );
    EXPECT_EQ( result2, expected2 );
}

TEST_F( PpuTest, ResolveNameTableAddress_FourScreen )
{
    u16 addr = 0x2FFF;
    u16 expected = addr & 0x0FFF;
    u16 result = ppu.ResolveNameTableAddress( addr, static_cast<int>( MirrorMode::FourScreen ) );
    EXPECT_EQ( result, expected );
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

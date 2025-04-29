#include "bus.h"
#include "cartridge.h"
#include "paths.h"
#include <fmt/base.h>
#include <gtest/gtest.h>

class PpuTest : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all
// tests
{
protected:
  // All tests assume flat memory model, which is why true is passed to Bus
  // constructor
  Bus        bus;
  PPU       &ppu = bus.ppu;
  CPU       &cpu = bus.cpu;
  Cartridge &cartridge = bus.cartridge;

  PpuTest()
  {
    std::string romFile = std::string( paths::roms() ) + "/palette.nes";
    cartridge.LoadRom( romFile );
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
  ppu.ppuStatus.bit.vBlank = 1;
  auto data = cpu.Read( 0x2002 );
  EXPECT_EQ( ppu.ppuStatus.bit.vBlank, 0 );
  EXPECT_EQ( data, 0x80 );
}

TEST_F( PpuTest, VramSetAddr )
{
  EXPECT_EQ( ppu.GetAddrLatch(), 0 );
  u8 data1 = 0x12;
  u8 data2 = 0x34;
  // First write
  cpu.Write( 0x2006, data1 );
  EXPECT_EQ( ppu.GetAddrLatch(), 1 );
  u16 tempAddr = 0x1200;
  EXPECT_EQ( ppu.GetTempAddr(), tempAddr );

  // Second write
  cpu.Write( 0x2006, data2 );
  EXPECT_EQ( ppu.GetAddrLatch(), 0 );
  u16 expectedAddr = 0x1234;
  EXPECT_EQ( ppu.GetTempAddr(), expectedAddr );
  EXPECT_EQ( ppu.vramAddr.value, expectedAddr );
}

TEST_F( PpuTest, VramReadDelay )
{
  // Write to vram address 0x2F00
  ppu.vramAddr.value = 0x2F00;
  bus.Write( 0x2007, 0x12 );
  bus.Write( 0x2007, 0x34 );

  // Vram address should have incremented by 2
  EXPECT_EQ( ppu.vramAddr.value, 0x2F02 );

  // Reading back should give the same values
  ppu.vramAddr.value = 0x2F00;
  bus.Read( 0x2007 ); // dummy read, data buffer delay
  auto data1 = bus.Read( 0x2007 );
  auto data2 = bus.Read( 0x2007 );
  EXPECT_EQ( data1, 0x12 );
  EXPECT_EQ( data2, 0x34 );
  EXPECT_EQ( ppu.vramAddr.value, 0x2F03 );
}

TEST_F( PpuTest, VramReadWrite )
{
  ppu.vramAddr.value = 0x2F00;
  bus.Write( 0x2007, 0x56 );
  ppu.vramAddr.value = 0x2F00;
  bus.Read( 0x2007 ); // delay
  auto data = bus.Read( 0x2007 );
  EXPECT_EQ( data, 0x56 );
}

TEST_F( PpuTest, VramBuffer )
{
  ppu.vramAddr.value = 0x2F00;
  bus.Write( 0x2007, 0x78 );
  ppu.vramAddr.value = 0x2F00;
  bus.Read( 0x2007 ); // dummy read
  // write a new value (0x12). This should not affect the buffered value.
  bus.Write( 0x2007, 0x12 );
  // read the buffered value. It should still be 0x78.
  auto bufferedValue = bus.Read( 0x2007 );
  EXPECT_EQ( bufferedValue, 0x78 );
}

TEST_F( PpuTest, VramBufferPaletteIsolation )
{
  ppu.vramAddr.value = 0x2F00;
  bus.Write( 0x2007, 0x9A );
  ppu.vramAddr.value = 0x2F00;
  bus.Read( 0x2007 ); // dummy read

  // write to a palette location (0x3F00), which should not affect the read
  // buffer.
  ppu.vramAddr.value = 0x3F00;
  bus.Write( 0x2007, 0x34 );

  // change back to a non-palette address to ensure buffer reads are re-enabled.
  ppu.vramAddr.value = 0x2F00;

  // read the buffered value. It should still be 0x9A.
  auto bufferedValue = bus.Read( 0x2007 );
  EXPECT_EQ( bufferedValue, 0x9A );
}

TEST_F( PpuTest, PaletteWriteRead )
{
  // Set palette entry at index 0 to 0x12.
  ppu.vramAddr.value = 0x3F00;
  bus.Write( 0x2007, 0x12 );
  ppu.vramAddr.value = 0x2F00;
  bus.Read( 0x2007 ); // fill buffer with dummy value
  ppu.vramAddr.value = 0x3F00;
  auto data = bus.Read( 0x2007 ); // should directly read the palette instead of the buffer
  EXPECT_EQ( data, 0x12 );
}

TEST_F( PpuTest, PaletteMirroring )
{
  // Write 0x12 to palette entry at index 0.
  ppu.vramAddr.value = 0x3F00;
  bus.Write( 0x2007, 0x12 );
  // Write 0x34 to palette entry at index 0xE0.
  // Since palette addresses are masked to 0x1F, index 0xE0 & 0x1F == 0x00.
  ppu.vramAddr.value = 0x3FE0;
  bus.Write( 0x2007, 0x34 );
  // Now read palette entry at index 0.
  auto data = ppu.GetPaletteEntry( 0x00 );
  // Expect that the mirror reflects the value from index 0xE0.
  EXPECT_EQ( data, 0x34 );
}
TEST_F( PpuTest, PaletteMirrorFrom10To00 )
{
  // Write 0x12 to palette entry at index 0.
  ppu.vramAddr.value = 0x3F00;
  bus.Write( 0x2007, 0x12 );
  // Write 0x34 to palette entry at index 0x10.
  // By masking, 0x10 maps to index 0x00.
  ppu.vramAddr.value = 0x3F10;
  bus.Write( 0x2007, 0x34 );
  auto data = ppu.GetPaletteEntry( 0x00 );
  EXPECT_EQ( data, 0x34 );
}

TEST_F( PpuTest, SpriteRamReadWrite )
{
  bus.Write( 0x2003, 0x00 );
  bus.Write( 0x2004, 0x12 );
  bus.Write( 0x2003, 0x00 );
  u8 data = bus.Read( 0x2004 );
  EXPECT_EQ( data, 0x12 );
}

TEST_F( PpuTest, SpriteWriteIncrement )
{
  bus.Write( 0x2003, 0x00 );
  bus.Write( 0x2004, 0x12 );
  bus.Write( 0x2004, 0x34 );
  bus.Write( 0x2003, 0x01 );
  u8 data = bus.Read( 0x2004 );
  EXPECT_EQ( data, 0x34 );
}

TEST_F( PpuTest, SpriteReadNoIncrement )
{
  bus.Write( 0x2003, 0x00 );
  bus.Write( 0x2004, 0x12 );
  bus.Write( 0x2004, 0x34 );
  bus.Write( 0x2003, 0x00 );
  u8 firstRead = bus.Read( 0x2004 );
  u8 secondRead = bus.Read( 0x2004 );
  EXPECT_EQ( firstRead, secondRead );
}

TEST_F( PpuTest, SpriteThirdByteMasked )
{
  bus.Write( 0x2003, 0x03 );
  bus.Write( 0x2004, 0xFF );
  bus.Write( 0x2003, 0x03 );
  u8 data = bus.Read( 0x2004 );
  EXPECT_EQ( data, 0xE3 );
}

TEST_F( PpuTest, SpriteDmaBasic )
{

  static const u8 testData[4] = { 0x12, 0x82, 0xE3, 0x78 };
  bus.Write( 0x200, 0x12 );
  bus.Write( 0x201, 0x82 );
  bus.Write( 0x202, 0xE3 );
  bus.Write( 0x203, 0x78 );

  // Set OAM address to 0.
  bus.Write( 0x2003, 0x00 );
  bus.Write( 0x4014, 0x02 );
  while ( bus.dmaInProgress ) {
    bus.Clock();
  }
  for ( int i = 0; i < 4; i++ ) {
    EXPECT_EQ( ppu.oam.data[i], testData[i] );
  }
}

TEST_F( PpuTest, SpriteDmaCopyWrap )
{
  static const u8 testData[4] = { 0x12, 0x82, 0xE3, 0x78 };

  // Write testData into CPU memory starting at address 0x200.
  bus.Write( 0x200, testData[0] );
  bus.Write( 0x201, testData[1] );
  bus.Write( 0x202, testData[2] );
  bus.Write( 0x203, testData[3] );

  // Set OAM address near the end, e.g. 0xFE.
  bus.Write( 0x2003, 0xFE );

  // Trigger DMA copy by writing 0x02 to $4014 (DMA source = 0x200).
  bus.Write( 0x4014, 0x02 );

  // Process DMA until it's finished.
  while ( bus.dmaInProgress ) {
    bus.Clock();
  }

  // Expected behavior with initial OAMADDR = 0xFE:
  // dmaOffset 0 -> destination = (0xFE + 0) mod 256 = 0xFE should receive
  // testData[0] dmaOffset 1 -> destination = (0xFE + 1) mod 256 = 0xFF should
  // receive testData[1] dmaOffset 2 -> destination = (0xFE + 2) mod 256 = 0x00
  // should receive testData[2] dmaOffset 3 -> destination = (0xFE + 3) mod 256
  // = 0x01 should receive testData[3]
  EXPECT_EQ( ppu.oam.data[0xFE], testData[0] );
  EXPECT_EQ( ppu.oam.data[0xFF], testData[1] );
  EXPECT_EQ( ppu.oam.data[0x00], testData[2] );
  EXPECT_EQ( ppu.oam.data[0x01], testData[3] );
}

TEST_F( PpuTest, SpriteDmaCopyPreservesOamAddr )
{
  // Set OAM address to 1.
  bus.Write( 0x2003, 0x01 );
  // Trigger DMA copy.
  bus.Write( 0x4014, 0x02 );
  while ( bus.dmaInProgress ) {
    bus.ProcessDma();
  }
  // Now write 0xFF to OAMDATA.
  bus.Write( 0x2004, 0xFF );
  // Reset OAM address to 1.
  bus.Write( 0x2003, 0x01 );
  u8 data = bus.Read( 0x2004 );
  EXPECT_EQ( data, 0xFF );
}

TEST_F( PpuTest, FetchNametableByte )
{
  ppu.vramAddr.value = 0x23AB;
  u16 testAddr = 0x2000 | ( ppu.vramAddr.value & 0x0FFF );
  ppu.WriteVram( testAddr, 0x12 );
  EXPECT_EQ( ppu.nametableByte, 0x00 );
  ppu.FetchNametableByte();
  EXPECT_EQ( ppu.nametableByte, 0x12 );
}

TEST_F( PpuTest, FetchAttributeByte )
{
  ppu.WriteVram( 0x23C0, 0xFF );
  ppu.WriteVram( 0x23FF, 0xAA ); // 10101010

  // Test (0,0) coarse coords (should fetch from 0x23C0)
  ppu.vramAddr.bit.coarseX = 0;
  ppu.vramAddr.bit.coarseY = 0;
  ppu.FetchAttributeByte();
  EXPECT_EQ( ppu.attributeByte, 0x03 ) << "Attribute byte should be 0x03 (lower 2 bits of 0xFF)";

  // Test (31, 29) coarse coords (should fetch from 0x23FF)
  ppu.vramAddr.bit.coarseX = 31;
  ppu.vramAddr.bit.coarseY = 29;
  ppu.FetchAttributeByte();
  EXPECT_EQ( ppu.attributeByte, 0x02 ) << "Attribute byte should be 0x02 (bits from 0xAA)";
}

TEST_F( PpuTest, FetchBgPatternBytes )
{
  u16 targetAddr = 0x0000;
  ppu.ppuCtrl.bit.patternBackground = 0; // 0x0000
  ppu.nametableByte = 0x00;              // tile 0
  ppu.vramAddr.bit.fineY = 0;            // row offset 0
  cartridge.SetChrROM( targetAddr, 0x55 );
  cartridge.SetChrROM( targetAddr + 8, 0xAA );

  ppu.FetchBgPattern0Byte();
  EXPECT_EQ( ppu.bgPattern0Byte, 0x55 ) << "Pattern low byte should be 0x55";
  ppu.FetchBgPattern1Byte();
  EXPECT_EQ( ppu.bgPattern1Byte, 0xAA ) << "Pattern high byte should be 0xAA";

  ppu.ppuCtrl.bit.patternBackground = 1; // 0x1000
  ppu.nametableByte = 0x10;              // tile 16
  ppu.vramAddr.bit.fineY = 1;            // row offset 1
  targetAddr = 0x1000 + ( 0x10 << 4 ) + 1;
  cartridge.SetChrROM( targetAddr, 0x12 );
  cartridge.SetChrROM( targetAddr + 8, 0x34 );

  ppu.FetchBgPattern0Byte();
  EXPECT_EQ( ppu.bgPattern0Byte, 0x12 ) << "Pattern low byte should be 0x12";
  ppu.FetchBgPattern1Byte();
  EXPECT_EQ( ppu.bgPattern1Byte, 0x34 ) << "Pattern high byte should be 0x34";
}

TEST_F( PpuTest, LoadShifters )
{
  ppu.bgPatternShiftLow = 0xFF00;
  ppu.bgPatternShiftHigh = 0xAA00;
  ppu.bgAttributeShiftLow = 0xFF12;
  ppu.bgAttributeShiftHigh = 0xAA34;
  ppu.bgPattern0Byte = 0x55;
  ppu.bgPattern1Byte = 0x33;

  // Attribute byte = 0b11 (both low and high mask)
  ppu.attributeByte = 0x03;

  ppu.LoadBgShifters();

  EXPECT_EQ( ppu.bgPatternShiftLow, 0xFF55 );
  EXPECT_EQ( ppu.bgPatternShiftHigh, 0xAA33 );
  EXPECT_EQ( ppu.bgAttributeShiftLow, 0xFFFF );
  EXPECT_EQ( ppu.bgAttributeShiftHigh, 0xAAFF );
}

TEST_F( PpuTest, IncrementCoarseX )
{
  cpu.Write( 0x2001, 0x08 ); // Rendering enabled

  // Test simple increment (0 → 1)
  ppu.vramAddr.bit.coarseX = 0;
  ppu.vramAddr.bit.nametableX = 0;
  ppu.IncrementCoarseX();
  EXPECT_EQ( ppu.vramAddr.bit.coarseX, 1 );
  EXPECT_EQ( ppu.vramAddr.bit.nametableX, 0 );

  // Test wrap-around (31 → 0, nametableX flips)
  ppu.vramAddr.bit.coarseX = 31;
  ppu.vramAddr.bit.nametableX = 0;
  ppu.IncrementCoarseX();
  EXPECT_EQ( ppu.vramAddr.bit.coarseX, 0 );
  EXPECT_EQ( ppu.vramAddr.bit.nametableX, 1 );

  // And back again (31 → 0, nametableX flips again)
  ppu.vramAddr.bit.coarseX = 31;
  ppu.vramAddr.bit.nametableX = 1;
  ppu.IncrementCoarseX();
  EXPECT_EQ( ppu.vramAddr.bit.coarseX, 0 );
  EXPECT_EQ( ppu.vramAddr.bit.nametableX, 0 );
}

TEST_F( PpuTest, IncrementCoarseY )
{
  cpu.Write( 0x2001, 0x08 ); // Rendering enabled

  // Fine Y < 7, should just increment fine Y
  ppu.vramAddr.bit.fineY = 6;
  ppu.IncrementCoarseY();
  EXPECT_EQ( ppu.vramAddr.bit.fineY, 7 );

  // Incrementing again should reset fine Y and increment coarse Y
  ppu.IncrementCoarseY();
  EXPECT_EQ( ppu.vramAddr.bit.fineY, 0 );
  EXPECT_EQ( ppu.vramAddr.bit.coarseY, 1 );

  // Coarse Y = 29, should flip nametable Y when fine y is 7 and increment
  // Non-attribute memory
  ppu.vramAddr.bit.coarseY = 29;
  ppu.vramAddr.bit.fineY = 7;
  ppu.vramAddr.bit.nametableY = 0;
  ppu.IncrementCoarseY();
  EXPECT_EQ( ppu.vramAddr.bit.coarseY, 0 );
  EXPECT_EQ( ppu.vramAddr.bit.fineY, 0 );
  EXPECT_EQ( ppu.vramAddr.bit.nametableY, 1 );

  // When in attribute memory, coarse Y should wrap back to zero when incremented from 31
  ppu.vramAddr.bit.coarseY = 31;
  ppu.vramAddr.bit.fineY = 7;
  ppu.vramAddr.bit.nametableY = 1;
  ppu.IncrementCoarseY();
  EXPECT_EQ( ppu.vramAddr.bit.coarseY, 0 );
  EXPECT_EQ( ppu.vramAddr.bit.fineY, 0 );
  EXPECT_EQ( ppu.vramAddr.bit.nametableY, 1 );
}

TEST_F( PpuTest, OddFrameSkip )
{
  ppu.Reset();

  // even, 340 ticks lands on cycle 340
  ppu.frame = 2;
  ppu.ppuMask.bit.renderBackground = 1;
  ppu.cycle = 0;
  for ( int i = 0; i < 340; i++ ) {
    ppu.Tick();
  }
  EXPECT_EQ( ppu.cycle, 340 );

  // odd, 340 ticks lands on cycle 0
  ppu.frame = 3;
  ppu.cycle = 0;
  for ( int i = 0; i < 340; i++ ) {
    ppu.Tick();
  }
  EXPECT_EQ( ppu.cycle, 0 );
}

TEST_F( PpuTest, VBlankPeriod )
{
  cpu.Write( 0x2001, 0x08 );

  // Enter VBlank
  ppu.scanline = 241;
  ppu.cycle = 1;
  ppu.Tick();
  EXPECT_EQ( ppu.ppuStatus.bit.vBlank, 1 );

  // Exit VBlank
  ppu.scanline = 261;
  ppu.cycle = 1;
  ppu.Tick();
  EXPECT_EQ( ppu.ppuStatus.bit.vBlank, 0 );
}

TEST_F( PpuTest, TransferAddressX )
{
  bus.Write( 0x2001, 0x08 ); // enable rendering
  ppu.cycle = 257;

  ppu.tempAddr.bit.nametableX = 1;
  ppu.tempAddr.bit.coarseX = 1;
  ppu.TransferAddressX();
  EXPECT_EQ( ppu.vramAddr.bit.nametableX, 1 );
  EXPECT_EQ( ppu.vramAddr.bit.coarseX, 1 );
}

TEST_F( PpuTest, IsSpriteInRange )
{
  // -- Test for normal 8-pixel sprite --
  {
    int  y = 20;
    bool isLarge = false; // Sprite height: 8 pixels, so valid scanlines: [20, 28)

    // Beginning of the range.
    EXPECT_TRUE( ppu.IsSpriteInRange( 20, y, isLarge ) );

    // A scanline in the middle.
    EXPECT_TRUE( ppu.IsSpriteInRange( 25, y, isLarge ) );

    // Last valid line (27 is < 28).
    EXPECT_TRUE( ppu.IsSpriteInRange( 27, y, isLarge ) );

    // Exactly at the boundary (28 is not less than 28).
    EXPECT_FALSE( ppu.IsSpriteInRange( 28, y, isLarge ) );

    // Before the range.
    EXPECT_FALSE( ppu.IsSpriteInRange( 19, y, isLarge ) );
  }

  // -- Test for large 16-pixel sprite --
  {
    int  y = 50;
    bool isLarge = true; // Sprite height: 16 pixels, so valid scanlines: [50, 66)

    // Beginning of the range.
    EXPECT_TRUE( ppu.IsSpriteInRange( 50, y, isLarge ) );

    // A scanline in the middle.
    EXPECT_TRUE( ppu.IsSpriteInRange( 55, y, isLarge ) );

    // Last valid line (65 is < 66).
    EXPECT_TRUE( ppu.IsSpriteInRange( 65, y, isLarge ) );

    // Exactly at the boundary (66 is not less than 66).
    EXPECT_FALSE( ppu.IsSpriteInRange( 66, y, isLarge ) );

    // Before the range.
    EXPECT_FALSE( ppu.IsSpriteInRange( 49, y, isLarge ) );
  }

  // -- Additional test for y=0 (edge case) --
  {
    int  y = 0;
    bool isLarge = false; // 8-pixel sprite: valid range: [0, 8)

    EXPECT_TRUE( ppu.IsSpriteInRange( 0, y, isLarge ) );  // at the top boundary
    EXPECT_TRUE( ppu.IsSpriteInRange( 7, y, isLarge ) );  // last valid scanline
    EXPECT_FALSE( ppu.IsSpriteInRange( 8, y, isLarge ) ); // one pixel too far
  }
}

int main( int argc, char **argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

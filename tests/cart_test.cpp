#include "bus.h"
#include "cartridge.h"
#include "paths.h"
#include <fmt/base.h>
#include <gtest/gtest.h>

class CartTest : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
protected:
  // All tests assume flat memory model, which is why true is passed to Bus constructor
  Bus  bus;
  PPU &ppu = bus.ppu;
  CPU &cpu = bus.cpu;

  CartTest()
  {
    std::string romFile = std::string( paths::roms() ) + "/palette.nes";
    bus.cartridge.LoadRom( romFile );
    bus.cpu.Reset();
  }
};

TEST_F( CartTest, iNes )
{
  // palette.nes
  std::array<char, 16> paletteHeader = { 0x4E, 0x45, 0x53, 0x1A, 0x02, 0x01, 0x00, 0x08,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  // Verfied with another header reader
  // Version: iNES 2.0
  // Mapper: 0 NROM, Sub 0
  // PRG ROM banks: 2 (32KB), PRG RAM banks: 0
  // CHR ROM banks: 1 (8KB),  CHR RAM banks: 0
  // Mirroring: 0 (Horizontal)
  // Region: 0 (NTSC)
  // System: Normal, VSSystemHardware: 0, VSSystemPPU: 0, ExtendSystem: 0
  // Trainer: 0
  // iNes 1.0 Unofficial Properties: 0
  // Misc Rom(s): 0
  iNes2Instance ines{};
  memcpy( ines.header.value, paletteHeader.data(), paletteHeader.size() );

  EXPECT_EQ( ines.header.value[0], 'N' );
  EXPECT_EQ( ines.header.value[1], 'E' );
  EXPECT_EQ( ines.header.value[2], 'S' );
  EXPECT_EQ( ines.header.value[3], 0x1A );
  EXPECT_EQ( ines.GetPrgRomBanks(), 2 );
  EXPECT_EQ( ines.GetChrRomBanks(), 1 );
  EXPECT_EQ( ines.GetMapper(), 0 );
  EXPECT_EQ( ines.GetMirroring(), 0 );
  EXPECT_EQ( ines.GetFourScreenMode(), 0 );
  EXPECT_EQ( ines.GetRegion(), 0 );
  EXPECT_EQ( ines.GetVsHardwareType(), 0 );
  EXPECT_EQ( ines.GetVsPpuType(), 0 );
  EXPECT_EQ( ines.GetDefaultExpansionDevice(), 0 );
  EXPECT_EQ( ines.GetMiscRoms(), 0 );
  EXPECT_EQ( ines.GetTrainerMode(), 0 );
  EXPECT_EQ( ines.GetBatteryMode(), 0 );
  EXPECT_EQ( ines.GetPrgRamSizeBytes(), 0 );
  EXPECT_EQ( ines.GetChrRamSizeBytes(), 0 );

  // instr_test-v5 header
  std::array<char, 16> v5header = { 0x4E, 0x45, 0x53, 0x1A, 0x10, 0x00, 0x10, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  // Version: iNES 1.0
  // Mapper: 1
  // PRG ROM banks: 16 (256KB), PRG RAM banks: 0, battery: 0
  // CHR ROM banks: 16 (128KB), CHR RAM banks: 0
  // Mirroring: Vertical, it's 0, but determined by the mapper
  // Region: 0 (NTSC)
  // System: Normal, VSSystemHardware: 0, VSSystemPPU: 0, ExtendSystem: 0
  memcpy( ines.header.value, v5header.data(), v5header.size() );
  EXPECT_EQ( ines.header.value[0], 'N' );
  EXPECT_EQ( ines.header.value[1], 'E' );
  EXPECT_EQ( ines.header.value[2], 'S' );
  EXPECT_EQ( ines.header.value[3], 0x1A );
  EXPECT_EQ( ines.GetPrgRomBanks(), 16 );
  EXPECT_EQ( ines.GetChrRomBanks(), 0 );
  EXPECT_EQ( ines.GetMapper(), 1 );
  EXPECT_EQ( ines.GetMirroring(), 0 );
  EXPECT_EQ( ines.GetFourScreenMode(), 0 );
  EXPECT_EQ( ines.GetRegion(), 0 );
  EXPECT_EQ( ines.GetVsHardwareType(), 0 );
  EXPECT_EQ( ines.GetVsPpuType(), 0 );
  EXPECT_EQ( ines.GetDefaultExpansionDevice(), 0 );
  EXPECT_EQ( ines.GetMiscRoms(), 0 );
  EXPECT_EQ( ines.GetTrainerMode(), 0 );
  EXPECT_EQ( ines.GetBatteryMode(), 0 );
  EXPECT_EQ( ines.GetPrgRamSizeBytes(), 0 );
  EXPECT_EQ( ines.GetChrRamSizeBytes(), 0 );
}

int main( int argc, char **argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

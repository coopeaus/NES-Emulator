#include "bus.h"
#include "cartridge.h"
#include "paths.h"
#include <fmt/base.h>
#include <gtest/gtest.h>

class StateTest : public ::testing::Test
{
protected:
  Bus        bus;
  PPU       &ppu = bus.ppu;
  CPU       &cpu = bus.cpu;
  Cartridge &cartridge = bus.cartridge;

  StateTest()
  {
    std::string romFile = std::string( paths::roms() ) + "/palette.nes";
    cartridge.LoadRom( romFile );
    bus.cpu.Reset();
  }
};

TEST_F( StateTest, CpuState )
{
  // clock the bus a few times
  for ( int i = 0; i < 10; ++i ) {
    bus.Clock();
  }

  /* these are the serialized values, we'll capture these as expcted values
    archive( pc,a, _x, _y, _s, _p, _cycles, _didVblank, _pageCrossPenalty, _isWriteModify, _reading2002,
             _instructionName,addrMode, _opcode, _isTestMode, _traceEnabled, _mesenFormatTraceEnabled, _didMesenTrace,
             _traceLog, _mesenFormatTraceLog );
   */
}

int main( int argc, char **argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

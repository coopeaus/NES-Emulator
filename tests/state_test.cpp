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

  auto pc = cpu.pc;
  auto a = cpu.a;
  auto x = cpu.x;
  auto y = cpu.y;
  auto s = cpu.s;
  auto p = cpu.p;
  auto cycles = cpu.cycles;
  auto didVblank = cpu.didVblank;
  auto pageCrossPenalty = cpu.pageCrossPenalty;
  auto isWriteModify = cpu.writeModify;
  auto reading2002 = cpu.reading2002;
  auto instructionName = cpu.instructionName;
  auto addrMode = cpu.addrMode;
  auto opcode = cpu.opcode;
  auto isTestMode = cpu.isTestMode;
  auto traceEnabled = cpu.traceEnabled;
  auto mesenFormatTraceEnabled = cpu.mesenFormatTraceEnabled;
  auto didMesenTrace = cpu.didMesenTrace;
  auto traceLog = cpu.traceLog;
  auto mesenFormatTraceLog = cpu.mesenFormatTraceLog;
}

int main( int argc, char **argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

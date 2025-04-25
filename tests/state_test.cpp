#include "bus.h"
#include "cartridge.h"
#include "paths.h"
#include <fmt/base.h>
#include <gtest/gtest.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/deque.hpp>

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
  // ─── Run a few clock cycles ───────────────────────────────────────────────
  for ( int i = 0; i < 10; ++i )
    bus.Clock();

  // ─── Capture "expected" state from cpu ────────────────────────────────────
  auto pc = cpu.pc;
  auto a = cpu.a;
  auto x = cpu.x;
  auto y = cpu.y;
  auto s = cpu.s;
  auto p = cpu.p;
  auto cycles = cpu.cycles;
  auto didVblank = cpu.didVblank;
  auto pageCrossPenalty = cpu.pageCrossPenalty;
  auto writeModify = cpu.writeModify;
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

  // ─── Serialize to an in‐memory buffer ──────────────────────────────────────
  std::stringstream ss( std::ios::binary | std::ios::in | std::ios::out );
  {
    cereal::BinaryOutputArchive outArchive( ss );
    outArchive( cpu );
  }

  // advance some more clocks to prove the archive really reset us
  for ( int i = 0; i < 10; ++i )
    bus.Clock();

  // rewind & clear before reading
  ss.clear();
  ss.seekg( 0 );

  {
    cereal::BinaryInputArchive inArchive( ss );
    inArchive( cpu );
  }

// ─── Macro to compare every field against the live 'cpu' ──────────────────
#define CPU_FIELDS                                                                                                     \
  X( pc )                                                                                                              \
  X( a )                                                                                                               \
  X( x )                                                                                                               \
  X( y )                                                                                                               \
  X( s )                                                                                                               \
  X( p )                                                                                                               \
  X( cycles )                                                                                                          \
  X( didVblank )                                                                                                       \
  X( pageCrossPenalty )                                                                                                \
  X( writeModify )                                                                                                     \
  X( reading2002 )                                                                                                     \
  X( instructionName )                                                                                                 \
  X( addrMode )                                                                                                        \
  X( opcode )                                                                                                          \
  X( isTestMode )                                                                                                      \
  X( traceEnabled )                                                                                                    \
  X( mesenFormatTraceEnabled )                                                                                         \
  X( didMesenTrace )                                                                                                   \
  X( traceLog )                                                                                                        \
  X( mesenFormatTraceLog )

#define X( field ) EXPECT_EQ( field, cpu.field );
  CPU_FIELDS
#undef X

  // ─── Now deserialize into a fresh Bus/CPU ─────────────────────────────────
  Bus  bus2;
  CPU &cpu2 = bus2.cpu;

  // reset buffer again
  ss.clear();
  ss.seekg( 0 );

  {
    cereal::BinaryInputArchive inArchive( ss );
    inArchive( cpu2 );
  }

// compare into cpu2
#define X( field ) EXPECT_EQ( field, cpu2.field );
  CPU_FIELDS
#undef X
#undef CPU_FIELDS
}

TEST_F( StateTest, BusState )
{
  // ─── Run a few clock cycles ───────────────────────────────────────────────
  for ( int i = 0; i < 10; ++i )
    bus.Clock();

  auto ppuCycle = ppu.cycle;
  auto scanline = ppu.scanline;
  auto cpuCycle = cpu.cycles;
  auto vramAddrValue = ppu.vramAddr.value;
  auto coarseX = ppu.vramAddr.bit.coarseX;
  auto oamData = ppu.oam.data;
  auto oamFirstEntryY = ppu.oam.entries.at( 0 ).y;

  bus.QuickSaveState();

  for ( int i = 0; i < 100000; ++i ) {
    bus.Clock();
  }

  bus.QuickLoadState();

  EXPECT_EQ( ppuCycle, ppu.cycle );
  EXPECT_EQ( scanline, ppu.scanline );
  EXPECT_EQ( cpuCycle, cpu.cycles );
  EXPECT_EQ( vramAddrValue, ppu.vramAddr.value );
  EXPECT_EQ( coarseX, ppu.vramAddr.bit.coarseX );
  EXPECT_EQ( oamData, ppu.oam.data );
  EXPECT_EQ( oamFirstEntryY, ppu.oam.entries.at( 0 ).y );
}

int main( int argc, char **argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

#include "bus.h"
#include "cartridge.h"
#include "paths.h"
#include "utils.h"
#include "global-types.h"

#include <filesystem>
#include <fstream>
#include <exception>
#include <functional>
#include <iostream>
// NOLINTBEGIN
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/memory.hpp>
// NOLINTEND
#include <string>
#include <stdexcept>
#include <system_error>

// Constructor to initialize the bus with a flat memory model
Bus::Bus() : cpu( this ), ppu( this ), cartridge( this )
{
}

/*
################################
||          CPU Read          ||
################################
*/
u8 Bus::Read( const u16 address, bool debugMode )
{
  if ( _useFlatMemory ) {
    return _flatMemory.at( address );
  }

  // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    return _ram.at( address & 0x07FF );
  }

  // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
  if ( address >= 0x2000 && address <= 0x3FFF ) {
    // ppu read will go here. For now, return from temp private member of bus
    const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
    return ppu.CpuRead( ppuRegister, debugMode );
  }

  // APU
  if ( address == 0x4015 ) {
    return apu.read_status();
  }

  // Controller read
  if ( address >= 0x4016 && address <= 0x4017 ) {
    auto data = ( controllerState[address & 0x0001] & 0x80 ) > 0;
    controllerState[address & 0x0001] <<= 1;
    return data;
  }

  // 4020 and up is cartridge territory
  if ( address >= 0x4020 && address <= 0xFFFF ) {
    return cartridge.Read( address );
  }

  // Unhandled address ranges return open bus value
  std::cout << "Unhandled read from address: " << std::hex << address << "\n";
  return 0xFF;
}

/*
################################
||          CPU Write         ||
################################
*/
void Bus::Write( const u16 address, const u8 data )
{

  if ( _useFlatMemory ) {
    _flatMemory.at( address ) = data;
    return;
  }

  // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    _ram.at( address & 0x07FF ) = data;
    return;
  }

  // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
  if ( address >= 0x2000 && address <= 0x3FFF ) {
    const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
    ppu.CpuWrite( ppuRegister, data );
    return;
  }

  // PPU DMA: 0x4014
  if ( address == 0x4014 ) {
    dmaInProgress = true;
    dmaAddr = data << 8;
    dmaOffset = 0;
    return;
  }

  // Controller input
  if ( address >= 0x4016 ) {
    controllerState[address & 0x0001] = controller[address & 0x0001];
    return;
  }

  // APU
  if ( utils::between( address, 0x4000, 0x4013 ) || address == 0x4015 || address == 0x4017 ) {
    apu.write_register( address, data );
    return;
  }

  // 4020 and up is cartridge territory
  if ( address >= 0x4020 && address <= 0xFFFF ) {
    cartridge.Write( address, data );
    return;
  }
  // Unhandled address ranges
  // Optionally log a warning or ignore
  std::cout << "Unhandled write to address: " << std::hex << address << "\n";
}

void Bus::ProcessDma()
{
  const u64 cycle = cpu.GetCycles();

  u8 const oamAddr = ppu.oamAddr;
  // Wait first read is on an odd cycle, wait it out.
  if ( dmaOffset == 0 && cycle % 2 == 1 ) {
    cpu.Tick();
    return;
  }

  // Read into OAM on even, load next byte on odd
  if ( cycle % 2 == 0 ) {
    auto data = Read( dmaAddr + dmaOffset );
    cpu.Tick();
    ppu.oam.data.at( ( oamAddr + dmaOffset ) & 0xFF ) = data;
    dmaOffset++;
  } else {
    dmaInProgress = dmaOffset < 256;
    cpu.Tick();
  }
}

void Bus::Clock()
{
  if ( dmaInProgress ) {
    ProcessDma();
  } else {
    cpu.DecodeExecute();
  }

  if ( ppu.nmiReady ) {
    ppu.nmiReady = false;
    cpu.NMI();
  }
}

/*
################################
||        Debug Methods       ||
################################
*/
[[nodiscard]] bool Bus::IsTestMode() const
{
  return _useFlatMemory;
}

void Bus::DebugReset()
{
  cpu.SetCycles( 0 );
  cpu.Reset();
  ppu.Reset();
}

/*
################################
||  Blargg's APU Integration  ||
################################
*/

int Bus::ReadDmc( void *objPtr, cpu_addr_t addr )
{
  Bus *self = static_cast<Bus *>( objPtr );
  return self->Read( addr );
}

void Bus::QuickSaveState( u8 idx )
{
  namespace fs = std::filesystem;
  fs::path const path = fs::path( paths::states() ) / cartridge.GetRomHash();

  if ( !fs::exists( path ) || !fs::is_directory( path ) )
    fs::create_directories( path );

  // filename format: save_slot0
  std::string const stateFilename = "save_slot" + std::to_string( idx ) + statefileExt;
  fs::path const    stateFilepath = path / stateFilename;
  SaveState( stateFilepath.string() );
}

void Bus::QuickLoadState( u8 idx )
{
  namespace fs = std::filesystem;
  fs::path const path = fs::path( paths::states() ) / cartridge.GetRomHash();

  if ( !fs::exists( path ) || !fs::is_directory( path ) )
    fs::create_directories( path );

  std::string const stateFilename = "save_slot" + std::to_string( idx ) + statefileExt;
  fs::path const    stateFilepath = path / stateFilename;
  LoadState( stateFilepath.string() );
}

void Bus::SaveState( const std::string &filename )
{
  try {
    std::ofstream outStream( filename, std::ios::out | std::ios::binary | std::ios::trunc );
    if ( !outStream ) {
      throw std::runtime_error( "Could not open '" + filename + "' for writing" );
    }

    cereal::BinaryOutputArchive archive( outStream );
    archive( *this );
  } catch ( const std::exception &e ) {
    std::cerr << "Error saving state: " << e.what() << "\n";
  }
}

void Bus::LoadState( const std::string &filename )
{
  try {
    std::ifstream inStream( filename, std::ios::in | std::ios::binary );
    if ( !inStream ) {
      throw std::runtime_error( "Could not open '" + filename + "' for reading" );
    }

    cereal::BinaryInputArchive archive( inStream );
    archive( *this );
  } catch ( const std::exception &e ) {
    std::cerr << "Error loading state: " << e.what() << "\n";
  }
}

bool Bus::DoesSaveSlotExist( int idx ) const
{
  namespace fs = std::filesystem;
  fs::path const hashDir = fs::path( paths::states() ) / cartridge.GetRomHash();
  if ( !( fs::exists( hashDir ) && fs::is_directory( hashDir ) ) ) {
    return false;
  }

  std::string const stateFilename = "save_slot" + std::to_string( idx ) + statefileExt;
  fs::path const    stateFilepath = hashDir / stateFilename;

  return fs::exists( stateFilepath ) && fs::is_regular_file( stateFilepath );
}

bool Bus::IsRomSignatureValid( const std::string &stateFile )
{
  namespace fs = std::filesystem;

  // Save current running state into a temp file
  fs::path const tmp =
      fs::temp_directory_path() / ( "bus_check_" + std::to_string( std::hash<std::string>{}( stateFile ) ) );
  SaveState( tmp.string() );

  // Check hashes for current and statefile
  const auto oldHash = cartridge.romHash;
  LoadState( stateFile );
  const auto newHash = cartridge.romHash;

  // Restore the original state
  LoadState( tmp.string() );

  // Clean up
  std::error_code ec;
  fs::remove( tmp, ec );

  return oldHash == newHash;
}

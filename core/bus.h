#pragma once
#include "global-types.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include <array>
#include <cstdint>

class Cartridge;
class CPU;
class PPU;
class APU;

class Bus
{
public:
  // Initialized with flat memory disabled by default. Enabled in json tests only
  Bus();

  template <class Archive> void serialize( Archive &ar ) // NOLINT
  {
    ar( cpu, ppu, apu, cartridge, dmaInProgress, dmaAddr, dmaOffset, controllerState, controller, _ram, _useFlatMemory,
        _flatMemory );
  }

  /*
  ################################
  ||         Peripherals        ||
  ################################
  */
  CPU       cpu;
  PPU       ppu;
  APU       apu;
  Cartridge cartridge;

  /*
  ################################
  ||         Bus Methods        ||
  ################################
  */
  u8   Read( uint16_t address, bool debugMode = false );
  void Write( u16 address, u8 data );
  void Clock();
  void ProcessDma();

  /*
  ################################
  ||    State Serialization     ||
  ################################
  */
  void QuickLoadState( u8 idx = 0 );
  void QuickSaveState( u8 idx = 0 );
  void SaveState( const std::string &filename );
  void LoadState( const std::string &filename );
  bool DoesSaveSlotExist( int idx = 0 ) const;
  bool IsRomSignatureValid( const std::string &stateFile );

  /*
  ################################
  ||      Global Variables      ||
  ################################
  */
  bool        dmaInProgress = false;
  u16         dmaAddr = 0x00;
  u16         dmaOffset = 0x00;
  u8          controllerState[2]{};
  u8          controller[2]{};
  std::string statefileExt = ".nesstate";

  /*
  ################################
  ||        Debug Methods       ||
  ################################
  */
  [[nodiscard]] bool IsTestMode() const;
  void               DebugReset();
  void               EnableJsonTestMode() { _useFlatMemory = true; }
  void               DisableJsonTestMode() { _useFlatMemory = false; }

private:
  /*
  ################################
  ||           CPU RAM          ||
  ################################
  */
  std::array<u8, 2048> _ram{}; // 2KB internal cpu RAM

  /*
  ################################
  ||       Debug Variables      ||
  ################################
  */
  bool                  _useFlatMemory{}; // For testing purposes
  std::array<u8, 65536> _flatMemory{};    // 64KB memory, for early testing
};

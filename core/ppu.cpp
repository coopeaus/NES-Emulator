#include "ppu.h"
#include "bus.h"
#include "cartridge.h" // NOLINT
#include "global-types.h"
#include "mappers/mapper-base.h"
#include <exception>
#include <array>
#include <iostream>

PPU::PPU( Bus *bus ) : bus( bus )
{
  try {
    LoadSystemPalette();
  } catch ( std::exception &e ) {
    std::cerr << e.what() << '\n';
    std::cerr << "Failed to load system palette from file.\n";
    std::cerr << "Using default palette.\n";
    failedPaletteRead = true;
    LoadDefaultSystemPalette();
  }
}

/*
################################
||                            ||
||       Handle CPU Read      ||
||                            ||
################################
*/
u8 PPU::CpuRead( u16 address, bool debugMode )
{

  /* @brief: CPU reads to the PPU
   * @details: Unlike other places in memory. Reads can cause PPU status updates.
   */

  // Non-readable registers: 2000 (PPUCTRL), 2001 (PPUMASK), 2003 (OAMADDR), 2005 (PPUSCROLL),
  // 2006 (PPUADDR)
  if ( !debugMode && ( isDisabled || address == 0x2000 || address == 0x2001 || address == 0x2003 || address == 0x2005 ||
                       address == 0x2006 ) ) {
    return 0xFF;
  }

  // 2002: PPU Status Register
  if ( address == 0x2002 ) {
    /*
     * Side Effects of Reading:
     *   1. Clears the vertical blank flag (Bit 7).
     *   2. Resets the PPU address latch for $2005/$2006.

     * The CPU expects the state of the PPU at the time of the read. The vertical
     * blank flag is cleared after constructing the return value, ensuring the
     * CPU sees the state before the read's side effects.
     */

    // Debug mode doesn't have side effects
    if ( debugMode ) {
      return ppuStatus.value;
    }

    u8 const data = ( ppuStatus.value & 0xE0 ) | ( vramBuffer & 0x1F );
    ppuStatus.bit.vBlank = 0;
    addrLatch = false;
    bus->cpu.SetReading2002( false );
    preventVBlank = false;
    return data;
  }
  // 2004: OAM Data
  if ( address == 0x2004 ) {
    /*
       Return the contents of oam[oamaddr]
       If called while renderingEnabled and in visible scanline range (0-239),
       Returns corrupted data (0xFF)
    */
    u8 value = oam.data.at( oamAddr & 0xFF );
    if ( debugMode ) {
      return value;
    }
    if ( IsRenderingEnabled() && scanline >= 0 && scanline <= 239 ) {
      return 0xFF;
    }
    if ( ( oamAddr & 0x03 ) == 3 ) {
      value &= 0xE3;
    }
    return value;
  }

  // 2007: PPU Data
  if ( address == 0x2007 ) {
    /*
      Reads the data at the current vramAddr
      When reading from palette memory, (0x3F00-0x3FFF), direct data is returned
      Otherwise, the data from the current vramBuffer is returned, simulating
      a delayed read by 1 cycle, and the vramAddr is then incremented by 1 or 32.
      1 if ppuCtrl increment mode is 0, 32 if 1.
      The vramBuffer is then updated with the data at the new vramAddr, in preparation
      for the next non-palette memory read.
     */
    // Debug mode has no side effects
    if ( debugMode ) {
      return vramBuffer;
    }

    u8 data = vramBuffer;
    vramBuffer = ReadVram( vramAddr.value );
    if ( vramAddr.value >= 0x3F00 && vramAddr.value <= 0x3FFF ) {
      data = vramBuffer;
    }

    vramAddr.value += ppuCtrl.bit.vramIncrement ? 32 : 1;
    return data;
  }
  return 0xFF;
}

/*
################################
||                            ||
||      Handle CPU Write      ||
||                            ||
################################
*/

void PPU::CpuWrite( u16 address, u8 data )
{
  /* @brief: CPU writes to the PPU
   */
  if ( isDisabled ) {
    return;
  }

  switch ( address ) {
    // 2000: PPUCTRL
    case 0x2000: // NOLINT
    {
      /*
        This write sets ppuCtrl to data.
        If ppuCtrl nmi is enabled and if ppuStatus vertical blank is set,
        initiates an non-maskable interrupt.
        Additionally, tempAddr nametable x and y bits are update from the
        cpuCtrl nametable x and y bits.
       */

      ppuCtrl.value = data;
      tempAddr.bit.nametableX = ppuCtrl.bit.nametableX;
      tempAddr.bit.nametableY = ppuCtrl.bit.nametableY;
      break;
    }
    // 2001: PPUMASK
    case 0x2001: {
      /*
        Sets ppuMask to data
        isRenderingEnabled can be determined here based on whether
        bgenabled and sprite enabled bits are set in the ppuMask
      */
      ppuMask.value = data;
      break;
    }
    // 2002: PPUSTATUS, does nothing
    case 0x2002: break;
    // 2003: OAMADDR
    case 0x2003: // NOLINT
    {
      oamAddr = data;
      break;
    }
    // 2004: OAMDATA
    case 0x2004: {
      /*
        Writes data to oam[oamAddr]
        Increments oamAddr
        If isRenderingEnabled and in visible scanline range (0-239),
        Ignore the write
       */
      if ( IsRenderingEnabled() && scanline >= 0 && scanline <= 239 ) {
        return;
      }
      oam.data.at( oamAddr & 0xFF ) = data;
      oamAddr = ( oamAddr + 1 ) & 0xFF;
      break;
    }
    // 2005: PPUSCROLL
    case 0x2005: // NOLINT
    {
      /*
         Updates the coarse x, y and fine x, y in two writes
         Coarse x, coarse y, and fine y are encoded into tempAddr
         Fine x has its own register, fineX
      */
      if ( !addrLatch ) // NOLINT
      {
        /* First Write
          Sets tempAddr coarse x from bits 3-7 of data (data & 0b11111000)
          Sets fineX from bits 0-2 of data (data & 0b00000111)
          Toggles addrLatch
         */
        fineX = data & 0x07;
        tempAddr.bit.coarseX = data >> 3;
        addrLatch = true;
      } else {
        /* Second Write
          Sets tempAddr fine y from bits 0-2 of data (data & 0b00000111)
          Sets tempAddr coarse y from bits 3-7 of data (data & 0b11111000)
          Toggles addrLatch
         */
        tempAddr.bit.fineY = data & 0x07;
        tempAddr.bit.coarseY = data >> 3;
        addrLatch = false;
      }
      break;
    }
    // 2006: PPUADDR
    case 0x2006: {
      /*
        Updates the high and low byte of the tempAddr in two writes
        High byte is written first
        Low byte is written second
       */
      if ( !addrLatch ) // NOLINT
      {
        /* First Write
           tempAddr high byte is derived from the first bits of data (data & 0b00111111)
           and is set to bits 8-14 of tempAddr
           The addrLatch is toggled
         */
        tempAddr.value = ( tempAddr.value & 0xFF ) | ( ( data & 0x3F ) << 8 );
        addrLatch = true;
      } else {
        /* Second Write
           The entire data byte is the tempAddr low byte
           tempAddr low byte is updated with the data byte
          tempAddr is copied to vramAddr
          addrLatch is toggled
         */
        tempAddr.value = ( tempAddr.value & 0xFF00 ) | data;
        vramAddr.value = tempAddr.value;
        addrLatch = false;
      }
      break;
    }
    // 2007: PPU Data
    case 0x2007: {
      /*
         data is written to the current vramAddr
        vramAddr is then incremented by 1 or 32.
        1 if ppuCtrl increment mode is 0, 32 if 1
      */
      //
      WriteVram( vramAddr.value, data );
      vramAddr.value += ppuCtrl.bit.vramIncrement ? 32 : 1;
    }
    default: break;
  }
}

/*
################################
||                            ||
||          PPU Read          ||
||                            ||
################################
*/

//------------------------------------------------------------------------------
// In your PPU class you still keep:
//   std::array<std::array<u8,0x400>,4> nameTables;
// but we will only ever use [0] and [1] in 2-table modes.
//------------------------------------------------------------------------------

u8 PPU::ReadVram( u16 address )
{
  // Mask to 14-bit range
  address &= 0x3FFF;

  // Pattern tables
  if ( address <= 0x1FFF ) {
    return bus->cartridge.Read( address );
  }

  // Nametables (0x2000–0x2FFF)
  if ( address >= 0x2000 && address <= 0x2FFF ) {
    // Mirror down into 0x000–0xFFF
    u16  v = address & 0x0FFF;
    u8   table = 0;
    auto m = GetMirrorMode();

    switch ( m ) {
      case MirrorMode::Vertical:
        //  0x000–0x3FF → NT0 0x400–0x7FF → NT1
        //  0x800–0xBFF → NT0 0xC00–0xFFF → NT1
        table = ( ( v >= 0x0000 && v <= 0x3FF ) || ( v >= 0x800 && v <= 0xBFF ) ) ? 0 : 1;
        break;
      case MirrorMode::Horizontal:
        //  0x000–0x3FF → NT0 0x400–0x7FF → NT0
        //  0x800–0xBFF → NT1 0xC00–0xFFF → NT1
        table = ( v < 0x800 ) ? 0 : 1;
        break;
      case MirrorMode::SingleLower: table = 0; break;
      case MirrorMode::SingleUpper: table = 1; break;
      case MirrorMode::FourScreen:
        // 0x000–0x3FF → NT0 0x400–0x7FF → NT1
        // 0x800–0xBFF → NT2 0xC00–0xFFF → NT3
        table = ( v / 0x400 ) & 0x03;
        break;
    }
    return nameTables.at( table ).at( v & 0x03FF );
  }

  // palettes
  // clang-format off
  if (address >= 0x3F00 && address <= 0x3FFF) {
    u8 idx = address & 0x1F;
    // Mirror the background palette entries
    if (idx == 0x10) idx = 0x00;
    if (idx == 0x14) idx = 0x04;
    if (idx == 0x18) idx = 0x08;
    if (idx == 0x1C) idx = 0x0C;
    return paletteMemory[idx] & 0x3F;
  }
  // clang-format on
  return 0xFF;
}

void PPU::WriteVram( u16 address, u8 data )
{
  address &= 0x3FFF;

  // Pattern tables
  if ( address <= 0x1FFF ) {
    bus->cartridge.Write( address, data );
    return;
  }

  // Nametables
  if ( address >= 0x2000 && address <= 0x2FFF ) {
    u16  v = address & 0x0FFF;
    u8   table = 0;
    auto m = GetMirrorMode();

    switch ( m ) {
      case MirrorMode::Vertical:
        table = ( ( v >= 0x0000 && v <= 0x3FF ) || ( v >= 0x800 && v <= 0xBFF ) ) ? 0 : 1;
        break;
      case MirrorMode::Horizontal : table = ( v < 0x800 ) ? 0 : 1; break;
      case MirrorMode::SingleLower: table = 0; break;
      case MirrorMode::SingleUpper: table = 1; break;
      case MirrorMode::FourScreen : table = ( v / 0x400 ) & 0x03; break;
    }
    nameTables.at( table ).at( v & 0x03FF ) = data;
    return;
  }

  // Palette
  // clang-format off
  if (address >= 0x3F00 && address <= 0x3FFF) {
    u8 idx = address & 0x1F;
    if (idx == 0x10) idx = 0x00;
    if (idx == 0x14) idx = 0x04;
    if (idx == 0x18) idx = 0x08;
    if (idx == 0x1C) idx = 0x0C;
    paletteMemory[idx] = data;
    return;
  }
  // clang-format on
}

/*
################################
||                            ||
||           Helpers          ||
||                            ||
################################
*/

void PPU::VBlank()
{
  if ( scanline == 241 ) {
    // If the CPU is reading register 2002 on cycle 0 of scanline 241
    // Vblank will not be set until the next frame due to a hardware race condition bug
    if ( cycle == 0 && bus->cpu.IsReading2002() ) {
      preventVBlank = true;
    }

    // Set the Vblank flag on cycle 1
    if ( cycle == 1 ) {

      if ( !preventVBlank ) {
        ppuStatus.bit.vBlank = 1;

        // Signal to trigger NMI if enabled
        if ( ppuCtrl.bit.nmiEnable ) {
          nmiReady = true;
        }
      }
      preventVBlank = false;
    }
  }
}

MirrorMode PPU::GetMirrorMode() const
{
  return bus->cartridge.GetMirrorMode();
}

/*
################################
||                            ||
||       PPU Cycle Tick       ||
||                            ||
################################
*/
void PPU::Tick()
{
  if ( isDisabled ) {
    return;
  }
  OddFrameSkip();

  if ( InScanline( 0, 239 ) )
    VisibleScanline();

  UpdateFrameBuffer();

  if ( scanline == 241 ) {
    VBlank();
    RenderFrameBuffer();
  }

  if ( scanline == 261 )
    PrerenderScanline();

  cycle++;

  if ( cycle > 340 ) {
    cycle = 0;
    scanline++;
    if ( scanline > 261 ) {
      scanline = 0;
      frame++;
    }
  }
}

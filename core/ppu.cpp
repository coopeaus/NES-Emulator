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
    if ( !debugMode && ( isDisabled || address == 0x2000 || address == 0x2001 || address == 0x2003 ||
                         address == 0x2005 || address == 0x2006 ) ) {
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
        case 0x2002:
            break;
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
        default:
            break;
    }
}

/*
################################
||                            ||
||          PPU Read          ||
||                            ||
################################
*/

[[nodiscard]] u8 PPU::ReadVram( u16 address )
{
    /*@brief: Internal PPU reads to the cartridge
     */
    address &= 0x3FFF;

    // $0000-$1FFF: Pattern Tables
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        /* Pattern table data is read from the cartridge */
        return bus->cartridge.Read( address );
    }

    // $2000-$3EFF: Name Tables
    if ( address >= 0x2000 && address <= 0x3EFF ) {
        /*
           Name table data is read from nameTables
          nameTables is a 2KiB array, with each name table taking up 1KiB
          Appropriate mirroing logic is handled in ResolveNameTableAddress

          If the mirroring mode is four screen, that means 4 name tables are
          used, each 1KiB in size. Since nameTables is only 2KiB, any address
          above 0x2800 must be directed to the cartridge, which provides the
          additional name table space. It's not necessary to emulate it this
          way, but it's a more accurate paradigm because that's how the hardware
          handles it.
        */
        address &= 0x2FFF;

        if ( GetMirrorMode() == MirrorMode::FourScreen && address >= 0x2800 ) {
            return bus->cartridge.ReadCartridgeVRAM( address );
        }

        u16 const nametableAddr = ResolveNameTableAddress( address );
        return nameTables.at( nametableAddr & 0x07FF );
    }

    // $3F00-$3FFF: Palettes
    if ( address >= 0x3F00 && address <= 0x3FFF ) {
        /*
        Background Palettes
        Palette 0: $3F00 (bg color), $3F01, $3F02, $3F03.
        Palette 1: $3F04 (bg color), $3F05, $3F06, $3F07.
        Palette 2: $3F08 (bg color), $3F09, $3F0A, $3F0B.
        Palette 3: $3F0C (bg color), $3F0D, $3F0E, $3F0F.

        Sprite Palettes
        Palette 4: $3F10 (mirrors 3F00), $3F11, $3F12, $3F13.
        Palette 5: $3F14 (mirrors 3F04), $3F15, $3F16, $3F17.
        Palette 6: $3F18 (mirrors 3F08), $3F19, $3F1A, $3F1B.
        Palette 7: $3F1C (mirrors 3F0C), $3F1D, $3F1E, $3F1F.

        The address is masked to 32 bytes.
        Addresses 3F10, 3F14, 3F18 and 3F1C mirror 3F00, 3F04, 3F08 and 3F0C respectively
        */
        address &= 0x1F;

        // Handle mirroring of 3F10, 3F14, 3F18, 3F1C to 3F00, 3F04, 3F08, 3F0C
        if ( address >= 0x10 && ( address & 0x03 ) == 0 ) {
            address &= 0x0F;
        }

        return paletteMemory.at( address ) & 0x3F;
    }

    return 0xFF;
}

/*
################################
||                            ||
||          PPU Write         ||
||                            ||
################################
*/
void PPU::WriteVram( u16 address, u8 data )
{
    /*@brief: Internal PPU reads to the cartridge
     */

    (void) data;
    address &= 0x3FFF;

    if ( address >= 0x0000 && address <= 0x1FFF ) {
        /* Pattern table data is written to the cartridge */
        bus->cartridge.Write( address, data );
        return;
    }
    if ( address >= 0x2000 && address <= 0x2FFF ) {

        /*
           Name table data is written to nameTables
          nameTables is a 2KiB array, with each name table taking up 1KiB
          Appropriate mirroing logic is handled in ResolveNameTableAddress

          If the mirroring mode is four screen, that means 4 name tables are
          used, each 1KiB in size. Since nameTables is only 2KiB, any address
          above 0x2800 must be directed to the cartridge, which provides the
          additional name table space.
        */

        address &= 0x2FFF;
        if ( GetMirrorMode() == MirrorMode::FourScreen && address >= 0x2800 ) {
            bus->cartridge.WriteCartridgeVRAM( address, data );
            return;
        }
        u16 const nametableAddr = ResolveNameTableAddress( address );
        nameTables.at( nametableAddr & 0x07FF ) = data;
        return;
    }

    // $3F00-$3FFF: Palettes
    if ( address >= 0x3F00 && address <= 0x3FFF ) {

        /* The address is masked to 32 bytes.
        Addresses 3F10, 3F14, 3F18 and 3F1C mirror 3F00, 3F04, 3F08 and 3F0C respectively
        */
        address &= 0x1F;

        // Handle mirroring of 3F10, 3F14, 3F18, 3F1C to 3F00, 3F04, 3F08, 3F0C
        if ( ( address & 0x03 ) == 0 ) {
            // write to both the mirrored and original address
            address &= 0x0F;
            paletteMemory.at( address ) = data;
            paletteMemory.at( address + 0x10 ) = data;
            return;
        }

        paletteMemory.at( address ) = data;
    }
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

// Called each cycle between cycle 65 and 256 to evaluate sprites.
void PPU::SpriteEvalForNextScanline()
{
    // Only run during visible scanlines (0–239) and cycles 65–256.
    if ( !IsRenderingEnabled() || scanline > 239 || cycle < 65 || cycle > 256 )
        return;

    // Start
    if ( cycle == 65 ) {
        SpriteEvalForNextScanlineStart();
    }

    if ( cycle & 0x01 ) // Odd cycle: perform a read.
    {
        oamCopyBuffer = oam.data.at( oamAddr );
    } else // Even cycle: process copying.
    {
        OamCopy();
    }
    oamAddr = ( spriteAddrLo & 0x03 ) | ( spriteAddrHi << 2 );

    // End
    if ( cycle == 256 ) {
        SpriteEvalForNextScanlineEnd();
    }
}

void PPU::OamCopy()
{
    if ( oamCopyDone ) {
        // If copying is already finished, simply advance the sprite address.
        spriteAddrHi = ( spriteAddrHi + 1 ) & 0x3F;
        if ( secondaryOamAddr >= 0x20 ) {
            oamCopyBuffer = secondaryOam.data.at( secondaryOamAddr & 0x1F );
        }
        return;
    }

    // Determine sprite height (8 or 16 pixels).
    u8 const spriteHeight = ( ppuCtrl.bit.spriteSize ? 16 : 8 );

    // Check if the sprite (Y coordinate in oamCopyBuffer) is in range for the next scanline.
    if ( !spriteInRange && ( ( scanline + 1 ) >= oamCopyBuffer ) &&
         ( ( scanline + 1 ) < ( oamCopyBuffer + spriteHeight ) ) ) {
        spriteInRange = true;
    }

    // If there's room in secondary OAM (32 bytes = 8 sprites * 4 bytes)
    if ( secondaryOamAddr < 0x20 ) {
        // Copy the current byte from primary OAM into secondary OAM.
        secondaryOam.data.at( secondaryOamAddr ) = oam.data.at( oamAddr );

        if ( spriteInRange ) {
            // Mark sprite 0 as added on cycle 66 if applicable.
            if ( cycle == 66 )
                sprite0Added = true;

            // Advance the low part of the sprite address and the secondary OAM pointer.
            spriteAddrLo++;
            secondaryOamAddr++;

            // When 4 bytes (a complete sprite entry) have been copied, move to the next sprite.
            if ( spriteAddrLo >= 4 ) {
                spriteAddrHi = ( spriteAddrHi + 1 ) & 0x3F;
                spriteAddrLo = 0;
                if ( spriteAddrHi == 0 )
                    oamCopyDone = true;
            }

            // Once a full sprite is copied, reset spriteInRange for the next sprite.
            if ( ( secondaryOamAddr & 0x03 ) == 0 )
                spriteInRange = false;
        } else {
            // Sprite not in range: skip copying and move to the next sprite.
            spriteAddrHi = ( spriteAddrHi + 1 ) & 0x3F;
            spriteAddrLo = 0;
            if ( spriteAddrHi == 0 )
                oamCopyDone = true;
        }
    } else {
        // Secondary OAM is full; skip to the next sprite.
        ppuStatus.bit.spriteOverflow = 1;
        spriteAddrHi = ( spriteAddrHi + 1 ) & 0x3F;
        spriteAddrLo = 0;
        if ( spriteAddrHi == 0 )
            oamCopyDone = true;
    }
}

u16 PPU::ResolveNameTableAddress( u16 addr, int testMirrorMode ) const
{

    MirrorMode mirrorMode = GetMirrorMode();

    // override mirror mode for testing
    if ( testMirrorMode != -1 ) {
        mirrorMode = static_cast<MirrorMode>( testMirrorMode );
    }

    switch ( mirrorMode ) {
        case MirrorMode::SingleUpper: // NOLINT
            // All addresses fall within 2000-23FF, nametable 0
            return 0x2000 | ( addr & 0x03FF );
        case MirrorMode::SingleLower:
            // All addresses fall within 2800-2BFF, nametable 2
            return 0x2800 | ( addr & 0x03FF );
        case MirrorMode::Vertical:
            /* Vertical Mirroring
              The two horizontal sections are unique, but the two vertical sections are mirrored
              2800-2FFF is a mirror of 2000-27FF

              2000 2400
              ^    ^
              v    v
              2800 2C00

              Horizontal scrolling games will use this mode. When screen data exceeds 27FF, it's
              wrapped back to 2000.
             */
            return 0x2000 | ( addr & 0x07FF );
        case MirrorMode::Horizontal:
            /* Horizontal Mirroring
              The two vertical sections are unique, but the two horizontal sections are mirrored
              2400-27FF is a mirror of 2000-23FF
              2C00-2FFF is a mirror of 2800-2BFF

              2000 < > 2400
              2800 < > 2C00

              Horizontal mode is used for vertical scrolling games, like Kid Icarus.

             Map addresses from 2C00-2FFF to 2800-2BFF if the address is for nametable 1
             Otherwise map addresses from 2400-27FF to 2000-23FF
             */

            if ( addr & 0x800 ) {
                return 0x2800 | ( addr & 0x03FF );
            } else {
                return 0x2000 | ( addr & 0x03FF );
            }

            return 0x2000 | ( ( addr & 0x03FF ) | ( ( addr & 0x800 ) >> 1 ) );
        case MirrorMode::FourScreen:
            /* Four-Screen Mirroring
               All four nametables are unique and backed by cartridge VRAM. There's no mirroring.
             */
            return addr & 0x0FFF;
        default:
            // Default to vertical mirroring
            return 0x2000 | ( addr & 0x07FF );
    }
    return 0xFF;
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

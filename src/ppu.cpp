#include "ppu.h"
#include "bus.h"
#include "utils.h"
#include "cartridge.h" // NOLINT
#include "mappers/mapper-base.h"

PPU::PPU( Bus *bus ) : _bus( bus )
{
    try {
        _nesPaletteRgbValues = utils::readPalette( "palettes/palette1.pal" );
    } catch ( std::exception &e ) {
        exit( EXIT_FAILURE );
    }
}

/*
################################
||                            ||
||       Handle CPU Read      ||
||                            ||
################################
*/
[[nodiscard]] u8 PPU::HandleCpuRead( u16 address ) // NOLINT
{

    /* @brief: CPU reads to the PPU
     * @details: Unlike other places in memory. Reads can cause PPU status updates.
     */

    // Non-readable registers: 2000 (PPUCTRL), 2001 (PPUMASK), 2003 (OAMADDR), 2005 (PPUSCROLL),
    // 2006 (PPUADDR)
    if ( _isDisabled || address == 0x2000 || address == 0x2001 || address == 0x2003 || address == 0x2005 ||
         address == 0x2006 ) {
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
        u8 const status = _ppuStatus.value & 0xE0;
        u8 const noise = _ppuStatus.value & 0x1F;
        u8 const data = status | noise;
        _ppuStatus.bit.verticalBlank = 0;
        _addrLatch = false;
        _bus->cpu.SetReading2002( false );
        _preventVBlank = false;
        return data;
    }
    // 2004: OAM Data
    if ( address == 0x2004 ) {
        /*
           Return the contents of _oam[_oamaddr]
           If called while _renderingEnabled and in visible scanline range (0-239),
           Returns corrupted data (0xFF)
        */
        if ( _isRenderingEnabled && _scanline >= 0 && _scanline <= 239 ) {
            return 0xFF;
        }
        return _oam[_oamAddr];
    }

    // 2007: PPU Data
    if ( address == 0x2007 ) {
        /*
          Reads the data at the current _vramAddr
          When reading from palette memory, (0x3F00-0x3FFF), direct data is returned
          Otherwise, the data from the current _ppuDataBuffer is returned, simulating
          a delayed read by 1 cycle, and the _vramAddr is then incremented by 1 or 32.
          1 if _ppuCtrl increment mode is 0, 32 if 1.
          The _ppuDataBuffer is then updated with the data at the new _vramAddr, in preparation
          for the next non-palette memory read.
         */
        if ( _vramAddr.value >= 0x3F00 && _vramAddr.value <= 0x3FFF ) {
            return Read( _vramAddr.value );
        }

        u8 const data = _ppuDataBuffer;
        _ppuDataBuffer = Read( _vramAddr.value );

        _vramAddr.value += _ppuCtrl.bit.vramIncrement ? 32 : 1;
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

void PPU::HandleCpuWrite( u16 address, u8 data ) // NOLINT
{
    /* @brief: CPU writes to the PPU
     */
    if ( _isDisabled ) {
        return;
    }

    (void) data; // NOTE: Remove after using data

    switch ( address ) {
        // 2000: PPUCTRL
        case 0x2000: // NOLINT
        {
            /*
              This write sets _ppuCtrl to data.
              If _ppuCtrl nmi is enabled and if _ppuStatus vertical blank is set,
              initiates an non-maskable interrupt.
              Additionally, _tempAddr nametable x and y bits are update from the
              _cpuCtrl nametable x and y bits.
             */
            // TODO: Implement
            break;
        }
        // 2001: PPUMASK
        case 0x2001: {
            /*
              Sets _ppuMask to data
              _isRenderingEnabled can be determined here based on whether
              bg_enabled and sprite_enabled bits are set in the _ppuMask
            */
            // TODO: Implement
            break;
        }
        // 2002: PPUSTATUS, does nothing
        case 0x2002:
            break;
        // 2003: OAMADDR
        case 0x2003: // NOLINT
        {
            /*
              Sets _oamAddr to data
             */
            // TODO: Implement
            break;
        }
        // 2004: OAMDATA
        case 0x2004: {
            /*
              Writes data to _oam[_oamAddr]
              Increments _oamAddr
              If _isRenderingEnabled and in visible scanline range (0-239),
              Ignore the write
             */
            if ( _isRenderingEnabled && _scanline >= 0 && _scanline <= 239 ) {
                return;
            }
            _oam[_oamAddr] = data;
            _oamAddr = ( _oamAddr + 1 ) & 0xFF;
            return;
        }
        // 2005: PPUSCROLL
        case 0x2005: // NOLINT
        {
            /*
               Updates the coarse x, y and fine x, y in two writes
               Coarse x, coarse y, and fine y are encoded into _tempAddr
               Fine x has its own register, _fineX
            */
            if ( !_addrLatch ) // NOLINT
            {
                /* First Write
                  Sets _tempAddr coarse x from bits 3-7 of data (data & 0b11111000)
                  Sets _fineX from bits 0-2 of data (data & 0b00000111)
                  Toggles _addrLatch
                 */
                // TODO: Implement
            } else {
                /* Second Write
                  Sets _tempAddr fine y from bits 0-2 of data (data & 0b00000111)
                  Sets _tempAddr coarse y from bits 3-7 of data (data & 0b11111000)
                  Toggles _addrLatch
                 */
                // TODO: Implement
            }
            break;
        }
        // 2006: PPUADDR
        case 0x2006: {
            /*
              Updates the high and low byte of the _tempAddr in two writes
              High byte is written first
              Low byte is written second
             */
            if ( !_addrLatch ) // NOLINT
            {
                /* First Write
                   _tempAddr high byte is derived from the first bits of data (data & 0b00111111)
                   and is set to bits 8-14 of _tempAddr
                   The _addrLatch is toggled
                 */
                // TODO: Implement
            } else {
                /* Second Write
                   The entire data byte is the _tempAddr low byte
                   _tempAddr low byte is updated with the data byte
                  _tempAddr is copied to _vramAddr
                  _addrLatch is toggled
                 */
                // TODO: Implement
            }
            break;
        }
        // 2007: PPU Data
        case 0x2007: {
            /*
               data is written to the current _vramAddr
              _vramAddr is then incremented by 1 or 32.
              1 if _ppuCtrl increment mode is 0, 32 if 1
            */
            // TODO: Implement
        }
        default:
            break;
    }
}

/*
################################
||                            ||
||           OAM DMA          ||
||                            ||
################################
*/

void PPU::DmaTransfer( u8 data ) // NOLINT
{
    /* @details CPU writes to address $4014 to initiate a DMA transfer
     * DMA transfer is a way to quickly transfer data from CPU memory to the OAM.
     * The CPU sends the starting address, N, to $4014, which is the high byte of the
     * source address
     * i.e. 0x02 -> 0x0200, 0x03 -> 0x0300, etc.
     *
     * Once the DMA transfer starts, the CPU reads 256 bytes sequentally from the address
     * into the OAM. The CPU is halted for 513/514 cycles during this time. Games usually
     * trigger this during a Vblank period
     *
     * Practically speaking, updating the OAM means updating the sprite information on screen
     *
     * This is not the only way to update the OAM, registers 2004 and 2003 can be used
     * but those are slower, and are used for partial updates mostly
     */
    // TODO: Implement
    (void) data;
}

/*
################################
||                            ||
||          PPU Read          ||
||                            ||
################################
*/

[[nodiscard]] u8 PPU::Read( u16 address ) // NOLINT
{
    /*@brief: Internal PPU reads to the cartridge
     */

    // $0000-$1FFF: Pattern Tables
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        /* Pattern table data is read from the cartridge */
        // TODO: Implement
    }

    // $2000-$3EFF: Name Tables
    if ( address >= 0x2000 && address <= 0x3EFF ) {
        /*
           Name table data is read from _nameTables
          _nameTables is a 2KiB array, with each name table taking up 1KiB
          Appropriate mirroing logic is handled in ResolveNameTableAddress

          If the mirroring mode is four screen, that means 4 name tables are
          used, each 1KiB in size. Since _nameTables is only 2KiB, any address
          above 0x2800 must be directed to the cartridge, which provides the
          additional name table space. It's not necessary to emulate it this
          way, but it's a more accurate paradigm because that's how the hardware
          handles it.

          // TODO: Implement
        */
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
        // TODO: Implement
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
void PPU::Write( u16 address, u8 data ) // NOLINT
{
    /*@brief: Internal PPU reads to the cartridge
     */

    (void) data;
    address &= 0x3FFF;

    if ( address >= 0x0000 && address <= 0x1FFF ) {
        /* Pattern table data is written to the cartridge */
        // TODO: Implement
    }
    if ( address >= 0x2000 && address <= 0x2FFF ) {

        /*
           Name table data is written to _nameTables
          _nameTables is a 2KiB array, with each name table taking up 1KiB
          Appropriate mirroing logic is handled in ResolveNameTableAddress

          If the mirroring mode is four screen, that means 4 name tables are
          used, each 1KiB in size. Since _nameTables is only 2KiB, any address
          above 0x2800 must be directed to the cartridge, which provides the
          additional name table space.
        */
        // TODO: Implement
    }

    // $3F00-$3FFF: Palettes
    if ( address >= 0x3F00 && address <= 0x3FFF ) {

        /* The address is masked to 32 bytes.
        Addresses 3F10, 3F14, 3F18 and 3F1C mirror 3F00, 3F04, 3F08 and 3F0C respectively
        */
        // TODO: Implement
    }
}

/*
################################
||                            ||
||       PPU Cycle Tick       ||
||                            ||
################################
*/
void PPU::Tick() // NOLINT
{
    if ( _isDisabled || _bus->IsTestMode() ) {
        return;
    }

    /*
    ################################
    ||    Odd Frame Cycle Skip    ||
    ################################
    */
    // TODO: Implement

    /*
    ################################
    ||       Increment Cycle      ||
    ################################
    */
    _cycle++;

    /*
    ################################
    ||       End Of Scanline      ||
    ################################
    */
    if ( _cycle > 340 ) {
        _cycle = 0;
        _scanline++;

        if ( _scanline > 260 ) {
            _scanline = -1;
            _frame++;
        }
    }

    /*
    ################################
    ||        Vblank Start        ||
    ################################
    */
    if ( _scanline == 241 ) {
        // If the CPU is reading register 2002 on cycle 0 of scanline 241
        // Vblank will not be set until the next frame due to a hardware race condition bug
        if ( _cycle == 0 && _bus->cpu.IsReading2002() ) {
            _preventVBlank = true;
        }

        // Set the Vblank flag on cycle 1
        if ( _cycle == 1 ) {

            // SDL Callback for rendering the framebuffer
            if ( onFrameReady != nullptr ) {
                onFrameReady( _frameBuffer.data() );
            }

            // Vblank and NMI
            if ( !_preventVBlank ) {
                _ppuStatus.bit.verticalBlank = 1;

                // Trigger NMI if NMI is enabled
                if ( _ppuCtrl.bit.nmiEnable ) {
                    TriggerNmi();
                }
            }
            _preventVBlank = false;
        }
    }

    /*
    ################################
    ||    Transfer Y (280-340)    ||
    ################################
    */
    // TODO: Implement

    /*
    ################################
    ||  Visible Scalines (0-239)  ||
    ################################
    */

    if ( _scanline < 0 || _scanline > 239 ) {
        return;
    }

    // Cycle 0, Idle cycle

    // Cycles 1-256: Tile and Pixel Rendering
    // Cycles 321-336 are beyond the visible scanline, but continue for the next scanline
    if ( ( _cycle >= 1 && _cycle <= 256 ) || ( _cycle >= 321 && _cycle <= 336 ) ) {
        u8 const stage = ( _cycle - 1 ) & 0x07;

        switch ( stage ) {
            // 0-1 fetch the nametable byte
            case 1:
                // Grab the first 12 bits of the vram address
                // which provide nametable select, coarse Y scroll, and coarse x scroll information
                // Nametable address is 0x2000 plus the offset of the vram address.
                // TODO: Implement
                break;

            // 2-3 fetch the attribute table byte
            case 3: // NOLINT
            {
                /* Attribute Table
                The attribute table is a 64-byte region located at addresses 0x23C0-0x23FF within the
                nametable memory. Each byte corresponds to a 32x32 pixel region on the screen,
                which is further divided into 4 smaller 16x16 pixel boxes.

                Each attribute byte determines the palette ID for each of these 16x16 pixel boxes,
                allowing the PPU to assign distinct color palettes to specific screen regions.

                Attribute Byte Structure:
                7654 3210
                |||| ||++- Palette ID for the top-left 16x16 pixel box
                |||| ++--- Palette ID for the top-right 16x16 pixel box
                ||++------ Palette ID for the bottom-left 16x16 pixel box
                ++-------- Palette ID for the bottom-right 16x16 pixel box

                Example:
                Attribute Byte: 1010 0111
                  |||| ||++- 11 (palette 3) for top-left box
                  |||| ++--- 01 (palette 1) for top-right box
                  ||++------ 10 (palette 2) for bottom-left box
                  ++-------- 10 (palette 2) for bottom-right box

                Visual Representation:
                  ,---- Top-left (palette 3)
                  |3 1 - Top-right (palette 1)
                  |2 2 - Bottom-right (palette 2)
                  `---- Bottom-left (palette 2)

                Each attribute byte affects a 32x32 pixel region. The attribute table covers the
                entire screen as an 8x8 grid of 32x32 regions (totaling 256x240 pixels).
                The first 8 bytes of the attribute table correspond to the top row of 32x256 pixels,
                the next 8 bytes correspond to the next row, and so on.

                Attribute Table Coverage:
                - Top-left corner of the screen: Address 0x23C0
                - Bottom-right corner of the screen: Address 0x23FF
                */

                // The attribute is 12 bits and is composed as follows
                /*
                NN 1111 YYY XXX
                || |||| ||| +++-- high 3 bits of coarse X (x/4)
                || |||| +++------ high 3 bits of coarse Y (y/4)
                || ++++---------- attribute offset (from the 23C0 offset)
                ++--------------- nametable select
                */
                // TODO: Implement
                break;
            }
            // 4-5 fetch pattern table plane 0
            case 5: {
                /*
                  If the name tables provide the "which tile" and "which palette",
                  the pattern table byte provides the "which pixel" and the "which palette color"

                  A pattern table byte represents an 8x8 tile.
                  Each bit is a 2bit color value (0-3). How can two bits fit into one bit?
                  They can't. That's why two planes are used.
                  Plane 0 holds the least significant bit of the color value
                  Plane 1 holds the most significant bit of the color value
                  A pattern table byte is grabbed in two reads, and the values are then combined

                  Here's an illustration with just 4 bits
                  Plane 1:   1010
                  Plane 0:   1100,

                  Index:     3120

                  Reading vertically, 0b11 = 3, 0b01 = 1, 0b10 = 2, 0b00 = 0
                  The 3, 1, 2, and 0 are the palette indeces for these pixels
                 */

                // TODO: Implement
                break;
            }
            // 6-7 fetch pattern table plane 1
            // 7: Increment scroll x
            // 7: Increment scroll y on cycle 256
            case 7: {
                // TODO: Implement
                break;
            }
            default:
                break;
        }
    }

    /*
    #################################
    ||  Transfer X Position (257)  ||
    #################################
    */
    // TODO: Implement

    /*
    ################################
    ||   Unused Reads (338, 340)  ||
    ################################
    */
    // TODO: Implement
}

/*
################################
||                            ||
||           Helpers          ||
||                            ||
################################
*/

u16 PPU::ResolveNameTableAddress( u16 addr )
{
    MirrorMode const mirrorMode = _bus->cartridge->GetMirrorMode();

    switch ( mirrorMode ) {
        case MirrorMode::SingleUpper: // NOLINT
            // All addresses fall within 2000-23FF, nametable 0
            // TODO: Implement
            break;
        case MirrorMode::SingleLower:
            // All addresses fall within 2800-2BFF, nametable 2
            // TODO: Implement
            break;
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
            return addr & 0x07FF;
        case MirrorMode::Horizontal:
            /* Horizontal Mirroring
              The two vertical sections are unique, but the two horizontal sections are mirrored
              2400-27FF is a mirror of 2000-23FF
              2C00-2FFF is a mirror of 2800-2BFF

              2000 < > 2400
              2800 < > 2C00

              Horizontal mode is used for vertical scrolling games, like Kid Icarus.
             */
            // TODO: Implement

        case MirrorMode::FourScreen:
            /* Four-Screen Mirroring
               All four nametables are unique and backed by cartridge VRAM. There's no mirroring.
             */
            return addr & 0x0FFF;
        default:
            // Default to vertical mirroring
            return addr & 0x07FF;
    }
    return 0xFF;
}

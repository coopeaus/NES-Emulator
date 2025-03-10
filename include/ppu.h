#pragma once
#include "cpu.h"
#include "mappers/mapper-base.h"
#include "utils.h"
#include <cstdint>
#include <array>
#include <functional>
#include <string>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;

using namespace std;

class PPU
{
  public:
    PPU( Bus *bus );

    /*
    ################################
    ||           Getters          ||
    ################################
    */
    MirrorMode GetMirrorMode() const;
    s16        GetScanline() const { return scanline; }
    u16        GetCycles() const { return cycle; }
    u64        GetFrame() const { return frame; }
    u32        GetMasterPaletteColor( u8 index ) const { return nesPaletteRgbValues.at( index ); }

    u8 GetPpuCtrl() const { return ppuCtrl.value; }
    u8 GetCtrlNametableX() const { return ppuCtrl.bit.nametableX; }
    u8 GetCtrlNametableY() const { return ppuCtrl.bit.nametableY; }
    u8 GetCtrlIncrementMode() const { return ppuCtrl.bit.vramIncrement; }
    u8 GetCtrlPatternSprite() const { return ppuCtrl.bit.patternSprite; }
    u8 GetCtrlPatternBackground() const { return ppuCtrl.bit.patternBackground; }
    u8 GetCtrlSpriteSize() const { return ppuCtrl.bit.spriteSize; }
    u8 GetCtrlNmiEnable() const { return ppuCtrl.bit.nmiEnable; }

    u8 GetPpuMask() const { return ppuMask.value; }
    u8 GetMaskGrayscale() const { return ppuMask.bit.grayscale; }
    u8 GetMaskShowBgLeft() const { return ppuMask.bit.renderBackgroundLeft; }
    u8 GetMaskShowSpritesLeft() const { return ppuMask.bit.renderSpritesLeft; }
    u8 GetMaskShowBg() const { return ppuMask.bit.renderBackground; }
    u8 GetMaskShowSprites() const { return ppuMask.bit.renderSprites; }
    u8 GetMaskEnhanceRed() const { return ppuMask.bit.enhanceRed; }
    u8 GetMaskEnhanceGreen() const { return ppuMask.bit.enhanceGreen; }
    u8 GetMaskEnhanceBlue() const { return ppuMask.bit.enhanceBlue; }

    u8 GetPpuStatus() const { return ppuStatus.value; }
    u8 GetStatusSpriteOverflow() const { return ppuStatus.bit.spriteOverflow; }
    u8 GetStatusSpriteZeroHit() const { return ppuStatus.bit.spriteZeroHit; }
    u8 GetStatusVblank() const { return ppuStatus.bit.verticalBlank; }

    u8 GetOamAddr() const { return oamAddr; }
    u8 GetOamData() const { return oamData; }
    u8 GetPpuScroll() const { return ppuScroll; }
    u8 GetPpuAddr() const { return ppuAddr; }
    u8 GetPpuData() const { return ppuData; }

    u16 GetVramAddr() const { return vramAddr.value; }
    u8  GetVramCoarseX() const { return vramAddr.bit.coarseX; }
    u8  GetVramCoarseY() const { return vramAddr.bit.coarseY; }
    u8  GetVramNametableX() const { return vramAddr.bit.nametableX; }
    u8  GetVramNametableY() const { return vramAddr.bit.nametableY; }
    u8  GetVramFineY() const { return vramAddr.bit.fineY; }
    u8  GetVramUnused() const { return vramAddr.bit.unused; }

    u16  GetTempAddr() const { return tempAddr.value; }
    u8   GetTempCoarseX() const { return tempAddr.bit.coarseX; }
    u8   GetTempCoarseY() const { return tempAddr.bit.coarseY; }
    u8   GetTempNametableX() const { return tempAddr.bit.nametableX; }
    u8   GetTempNametableY() const { return tempAddr.bit.nametableY; }
    u8   GetTempFineY() const { return tempAddr.bit.fineY; }
    u8   GetTempUnused() const { return tempAddr.bit.unused; }
    u8   GetFineX() const { return fineX; }
    bool GetAddrLatch() const { return addrLatch; }

    u16 GetBgShiftPatternLow() const { return bgShiftPatternLow; }
    u16 GetBgShiftPatternHigh() const { return bgShiftPatternHigh; }
    u16 GetBgShiftAttributeLow() const { return bgShiftAttributeLow; }
    u16 GetBgShiftAttributeHigh() const { return bgShiftAttributeHigh; }

    /*
    ################################
    ||           Setters          ||
    ################################
    */
    void SetScanline( s16 val ) { scanline = val; }
    void SetCycles( u16 cycles ) { cycle = cycles; }

    /*
    ################################
    ||      CPU Read / Write      ||
    ################################
    */
    [[nodiscard]] u8 HandleCpuRead( u16 address, bool debugMode = false );
    void             HandleCpuWrite( u16 address, u8 data );

    /*
    ################################
    ||       Internal Reads       ||
    ################################
    */
    [[nodiscard]] u8 Read( u16 addr );

    /*
    ################################
    ||       Internal Writes      ||
    ################################
    */
    void Write( u16 addr, u8 data );

    /*
    ################################
    ||         PPU Methods        ||
    ################################
    */
    void DmaTransfer( u8 data );
    u16  ResolveNameTableAddress( u16 addr, int testMirrorMode = -1 ) const;
    void Tick();
    void LoadNextBgShiftRegisters();
    void UpdateShiftRegisters();
    void LoadNametableByte();
    void LoadAttributeByte();
    void LoadPatternPlane0Byte();
    void LoadPatternPlane1Byte();
    void IncrementScrollX();
    void IncrementScrollY();
    u8   GetBgPalette();
    u8   GetSpritePalette();
    u8   GetBgPixel();
    u8   GetSpritePixel();
    u32  GetOutputPixel( u8 bgPixel, u8 spritePixel, u8 bgPalette, u8 spritePalette );
    void TriggerNmi() const;
    void Reset()
    {
        scanline = 0;
        cycle = 4;
        frame = 1;
        isRenderingEnabled = false;
        preventVBlank = false;
        ppuCtrl.value = 0x00;
        ppuMask.value = 0x00;
        ppuStatus.value = 0x00;
        oamAddr = 0x00;
        oamData = 0x00;
        ppuScroll = 0x00;
        ppuAddr = 0x00;
        ppuData = 0x00;
        addrLatch = false;
        ppuDataBuffer = 0x00;
        vramAddr.value = 0x0000;
        tempAddr.value = 0x0000;
        fineX = 0x00;
        nameTables.fill( 0x00 );
        paletteMemory = defaultPalette;
    }
    void IncrementSystemPalette()
    {
        if ( failedPaletteRead ) {
            return;
        }
        systemPaletteIdx = ( systemPaletteIdx + 1 ) % maxSystemPalettes;
        LoadSystemPalette( systemPaletteIdx );
    }
    void DecrementSystemPalette()
    {
        if ( failedPaletteRead ) {
            return;
        }
        int newIdx = systemPaletteIdx - 1;
        if ( newIdx < 0 ) {
            newIdx = maxSystemPalettes - 1;
        }
        systemPaletteIdx = newIdx;
        LoadSystemPalette( systemPaletteIdx );
    }

    void LoadSystemPalette( int paletteIdx = 0 )
    {
        std::string const palettePath = systemPalettePaths.at( paletteIdx );
        nesPaletteRgbValues = utils::readPalette( palettePath );
    }
    u8  GetPpuPaletteValue( u8 index ) { return paletteMemory.at( index ); }
    u32 GetPpuPaletteColor( u8 index ) { return nesPaletteRgbValues.at( paletteMemory.at( index ) ); }

    void LoadDefaultSystemPalette()
    {
        // clang-format off
        nesPaletteRgbValues = {
            0xFF606060, 0xFF7B2100, 0xFF9C0000, 0xFF8B0031, 0xFF6F0059, 0xFF31006F, 0xFF000064, 0xFF00114F,
            0xFF00192F, 0xFF002927, 0xFF004400, 0xFF373900, 0xFF4F3900, 0xFF000000, 0xFF0C0C0C, 0xFF0C0C0C,
            0xFFAEAEAE, 0xFFCE5610, 0xFFFF2C1B, 0xFFEC2060, 0xFFBF00A9, 0xFF5416CA, 0xFF0800CA, 0xFF043A9E,
            0xFF005167, 0xFF006143, 0xFF007C00, 0xFF537100, 0xFF877100, 0xFF0C0C0C, 0xFF0C0C0C, 0xFF0C0C0C,
            0xFFFFFFFF, 0xFFFE9E44, 0xFFFF6C5C, 0xFFFF6699, 0xFFFF60D7, 0xFF9562FF, 0xFF5364FF, 0xFF3094F4,
            0xFF00ACC2, 0xFF14C490, 0xFF28D252, 0xFF92C620, 0xFFD2BA18, 0xFF4C4C4C, 0xFF0C0C0C, 0xFF0C0C0C,
            0xFFFFFFFF, 0xFFFFCCA3, 0xFFFFB4A4, 0xFFFFB6C1, 0xFFFFB7E0, 0xFFC5C0FF, 0xFFABBCFF, 0xFF9FD0FF,
            0xFF90E0FC, 0xFF98EAE2, 0xFFA0F2CA, 0xFFE2EAA0, 0xFFFAE2A0, 0xFFB6B6B6, 0xFF0C0C0C, 0xFF0C0C0C
        };
        // clang-format on
    }

    /*
    ################################
    ||        SDL Callback        ||
    ################################
    */
    // void ( *onFrameReady )( const u32 *frameBuffer ) = nullptr;
    std::function<void( const u32 * )> onFrameReady = nullptr;

    /*
    ################################
    ||        Debug Methods       ||
    ################################
    */
    void EnableJsonTestMode() { isDisabled = true; }
    void DisableJsonTestMode() { isDisabled = false; }

    // Get pattern table data, used in debugging (pattern-tables.h)
    std::array<u32, 16384> GetPatternTable( int tableIdx )
    {
        std::array<u32, 16384> buffer{};
        u16 const              baseAddr = tableIdx == 0 ? 0x0000 : 0x1000;

        // 256 tiles in a pattern table
        for ( int tile = 0; tile < 256; tile++ ) {
            int const tileX = tile % 16;
            int const tileY = tile / 16;

            // 16 bytes per tile, split into two bit planes
            u16 const tileAddr = baseAddr + ( tile * 16 );

            // Each tile is 8x8 pixels
            for ( int row = 0; row < 8; row++ ) {
                u8 const plane0Byte = Read( tileAddr + row );
                u8 const plane1Byte = Read( tileAddr + row + 8 );
                for ( int bit = 7; bit >= 0; bit-- ) {
                    u8 const plane0Bit = ( plane0Byte >> bit ) & 0x01;
                    u8 const plane1Bit = ( plane1Byte >> bit ) & 0x01;
                    u8 const colorIdx = ( plane1Bit << 1 ) | plane0Bit;

                    // Calculate the buffer index (pixel position)
                    int const localX = 7 - bit;
                    int const globalX = ( tileX * 8 ) + localX;
                    int const globalY = ( tileY * 8 ) + row;
                    int const bufferIdx = ( globalY * 128 ) + globalX;
                    buffer.at( bufferIdx ) = GetPpuPaletteColor( colorIdx );
                }
            }
        }

        return buffer;
    }

    // Nametable data, used in debugging (nametables.h)
    std::array<u32, 61440> GetNametable( int nametableIdx )
    {
        std::array<u32, 61440> buffer{};

        u16 vramStart = 0x2000;
        switch ( nametableIdx ) {
            case 0:
                vramStart = 0x2000;
                break;
            case 1:
                vramStart = 0x2400;
                break;
            case 2:
                vramStart = 0x2800;
                break;
            case 3:
                vramStart = 0x2C00;
                break;
            default:
                break;
        }

        /* Vram Structure, for reference
        yyy NN YYYYY XXXXX
        ||| || ||||| +++++-- tile X (coarse X scroll)
        ||| || +++++-------- tile Y (coarse Y scroll)
        ||| ++-------------- nametable 0-3
        +++----------------- fine Y scroll
        */

        u16 const vramEnd = vramStart + 960;
        u16 const attrBase = vramStart + 960;

        for ( int vramAddr = vramStart; vramAddr < vramEnd; vramAddr++ ) {

            // Determines which 8x8 tile is being processed, which is info provided by the vram addr
            const int tileX = vramAddr & 0x1F;
            const int tileY = ( vramAddr >> 5 ) & 0x1F;

            // Bit 4 from ctrl registers determines the pattern table
            u16 const patternTableBaseAddr = ppuCtrl.bit.patternBackground ? 0x1000 : 0x0000;

            // Vram address determines which tile index to use from the pattern table (0-255)
            u8 const tileIndex = Read( vramAddr );

            // Combining the two gives the pattern table address
            u16 const tileAddr = patternTableBaseAddr + ( tileIndex * 16 );

            // Grab the attribute byte, which covers a 32x32 area
            u8 const  attrX = tileX / 4;
            u8 const  attrY = tileY / 4;
            u16 const attrAddr = attrBase + ( attrY * 8 ) + attrX;
            u8 const  attributeByte = Read( attrAddr );

            // Determine which 4x4 quadrant the tile is in
            u8 const attrQuadX = ( tileX % 4 ) >> 1;
            u8 const attrQuadY = ( tileY % 4 ) >> 1;
            // (0,0) -> 0, (0,1) -> 1, (1,0) -> 2, (1,1) -> 3
            u8 const quadrant = ( attrQuadY << 1 ) | attrQuadX;

            // Extract the palette index from the attribute byte
            /*
                7654 3210
                |||| ||++- Color bits 3-2 for top left quadrant
                |||| ++--- Color bits 3-2 for top right quadrant
                ||++------ Color bits 3-2 for bottom left quadrant
                ++-------- Color bits 3-2 for bottom right quadrant
            */
            u8 const paletteIdx = ( attributeByte >> ( 2 * quadrant ) ) & 0x03;

            // Now, combining all the tile data and adding it to the correct location in the buffer
            for ( int pixelRow = 0; pixelRow < 8; pixelRow++ ) {
                u8 const plane0Byte = Read( tileAddr + pixelRow );
                u8 const plane1Byte = Read( tileAddr + pixelRow + 8 );
                for ( int bit = 7; bit >= 0; bit-- ) {
                    u8 const plane0Bit = ( plane0Byte >> bit ) & 0x01;
                    u8 const plane1Bit = ( plane1Byte >> bit ) & 0x01;
                    u8 const colorIdx = ( plane1Bit << 1 ) | plane0Bit;

                    // Calculate the buffer index (final pixel position)
                    int const tilePixelX = 7 - bit;
                    int const screenPixelX = ( tileX * 8 ) + tilePixelX;
                    int const screenPixelY = ( tileY * 8 ) + pixelRow;
                    int const bufferIdx = ( screenPixelY * 256 ) + screenPixelX;
                    u8 const  finalColorIdx = ( paletteIdx * 4 ) + colorIdx;
                    buffer.at( bufferIdx ) = GetPpuPaletteColor( finalColorIdx );
                }
            }
        }
        return buffer;
    }

    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
    bool failedPaletteRead = false;
    int  systemPaletteIdx = 0;
    int  maxSystemPalettes = 3;

    std::array<std::string, 3> systemPalettePaths = { "palettes/palette1.pal", "palettes/palette2.pal",
                                                      "palettes/palette3.pal" };

    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
    s16            scanline = 0;
    u16            cycle = 4;
    u64            frame = 1;
    bool           isRenderingEnabled = false;
    bool           preventVBlank = false;
    array<u32, 64> nesPaletteRgbValues{};

    /*
    ################################
    ||        SDL Variables       ||
    ################################
    */
    array<u32, 61440> frameBuffer{};

    /*
    ################################
    ||       Debug Variables      ||
    ################################
    */
    bool isDisabled = false;

    /*
    ######################################
    ||  Background Rendering Variables  ||
    ######################################
    */
    u8  nametableByte = 0x00;
    u8  attributeByte = 0x00;
    u8  bgPlane0Byte = 0x00;
    u8  bgPlane1Byte = 0x00;
    u16 bgShiftPatternLow = 0x0000;
    u16 bgShiftPatternHigh = 0x0000;
    u16 bgShiftAttributeLow = 0x0000;
    u16 bgShiftAttributeHigh = 0x0000;

    /*
    ################################
    ||         Peripherals        ||
    ################################
    */
    Bus *bus;

    /*
    ################################
    ||    CPU-facing Registers    ||
    ################################
    */

    union PPUCTRL {
        struct {
            u8 nametableX : 1;
            u8 nametableY : 1;
            u8 vramIncrement : 1;
            u8 patternSprite : 1;
            u8 patternBackground : 1;
            u8 spriteSize : 1;
            u8 slaveMode : 1; // unused
            u8 nmiEnable : 1;
        } bit;
        u8 value = 0x00;
    };
    union PPUMASK {
        struct {
            u8 grayscale : 1;
            u8 renderBackgroundLeft : 1;
            u8 renderSpritesLeft : 1;
            u8 renderBackground : 1;
            u8 renderSprites : 1;
            u8 enhanceRed : 1;
            u8 enhanceGreen : 1;
            u8 enhanceBlue : 1;
        } bit;
        u8 value = 0x00;
    };
    union PPUSTATUS {
        struct {
            u8 unused : 5;
            u8 spriteOverflow : 1;
            u8 spriteZeroHit : 1;
            u8 verticalBlank : 1;
        } bit;
        u8 value = 0x00;
    };

    PPUCTRL   ppuCtrl;          // $2000
    PPUMASK   ppuMask;          // $2001
    PPUSTATUS ppuStatus;        // $2002
    u8        oamAddr = 0x00;   // $2003
    u8        oamData = 0x00;   // $2004
    u8        ppuScroll = 0x00; // $2005
    u8        ppuAddr = 0x00;   // $2006
    u8        ppuData = 0x00;   // $2007
    // $4014: OAM DMA, handled in bus read/write, see bus.cpp

    /*
    ################################################################
    ||                     Internal Registers                     ||
    ################################################################
    */
    union LoopyRegister {
        struct {
            u16 coarseX : 5;
            u16 coarseY : 5;
            u16 nametableX : 1;
            u16 nametableY : 1;
            u16 fineY : 3;
            u16 unused : 1;
        } bit;
        u16 value = 0x00;
    };
    /* v: Current VRAM address (15 bits)
       t: Temporary VRAM address (15 bits)

      The v (and t) register has multiple purposes
      - It allows the CPU to write to the PPU memory through _ppuAddr and _ppuData registers
      - It points to the nametable data currently being drawn

        yyy NN YYYYY XXXXX
        ||| || ||||| +++++-- coarse X scroll
        ||| || +++++-------- coarse Y scroll
        ||| ++-------------- nametable select
        +++----------------- fine Y scroll

        There's also a temporary VRAM address, t, which is identical to v and is used to store
        data temporarily until it is copied to v.

        Both of these registers are sometimes referred to as "Loopy Registers", named
        after the developer who discovered how they work.
     */
    LoopyRegister vramAddr;
    LoopyRegister tempAddr;

    // Internal fine X scroll register
    u8 fineX = 0x00;

    // Used by _ppuScroll and _ppuAddr for two-write operations
    bool addrLatch = false;

    // Stores last data written to _ppuData
    u8 ppuDataBuffer = 0x00;

    /*
    ################################################################
    ||                  Internal Memory Locations                 ||
    ################################################################
    */
    array<u8, 2048> nameTables{};

    /* Pattern Tables
        $0000-$0FFF: Pattern Table 1
        $1000-$1FFF: Pattern Table 2
        Defined and documented in cartridge.h
    */

    /*
       Palette Memory
      $3F00-$3F0F: Background Palettes
      $3F10-$3F1F: Sprite Palettes

      A palette is a group of 4 indices, each index ranging from 0-63
      The NES has 64 fixed colors, so each index represents a color

      The colors won't be defined here, but somewhere in the SDL
      rendering logic. We'll use .pal files to easily define and swap fixed colors
      It's worth documenting what the palettes are, as this concept can be confusing

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

      Sprite backgrounds, despite being mirrored, are ignored and treated
      as transparent.
    */
    // Default boot palette, will get changed by the cartridge
    // clang-format off
    array<u8, 32> defaultPalette = {
        0x09, 0x01, 0x00, 0x01, 
        0x00, 0x02, 0x02, 0x0D,
        0x08, 0x10, 0x08, 0x24,
        0x00, 0x00, 0x04, 0x2C,
        0x09, 0x01, 0x34, 0x03,
        0x00, 0x04, 0x00, 0x14,
        0x08, 0x3A, 0x00, 0x02,
        0x00, 0x20, 0x2C, 0x08 
    };

    // clang-format on
    array<u8, 32> paletteMemory = defaultPalette;

    /* Object Attribute Memory (OAM)
       This is a 256 byte region internal to the PPU
       It holds metadata for up to 64 sprites, with each sprite taking up 4 bytes.

      Byte 0: Y position of the sprite
      Byte 1: Tile index number
        76543210
        ||||||||
        |||||||+- Bank ($0000 or $1000) of tiles
        +++++++-- Tile number of top of sprite (0 to 254; bottom half gets the next tile)
      Byte 2: Attributes
        76543210
        ||||||||
        ||||||++- Palette (4 to 7) of sprite
        |||+++--- Unimplemented (read 0)
        ||+------ Priority (0: in front of background; 1: behind background)
        |+------- Flip sprite horizontally
        +-------- Flip sprite vertically
      Byte 3: X position of the sprite
    */
    array<u8, 256> oam{};
};

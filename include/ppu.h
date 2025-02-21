#pragma once
#include "cpu.h"
#include "mappers/mapper-base.h"
#include <cstdint>
#include <array>
#include <functional>

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
    [[nodiscard]] MirrorMode GetMirrorMode();
    [[nodiscard]] s16        GetScanline() const { return _scanline; }
    [[nodiscard]] u16        GetCycles() const { return _cycle; }
    [[nodiscard]] u64        GetFrame() const { return _frame; }

    /*
    ################################
    ||           Setters          ||
    ################################
    */
    void SetScanline( s16 scanline ) { _scanline = scanline; }
    void SetCycles( u16 cycles ) { _cycle = cycles; }

    /*
    ################################
    ||      CPU Read / Write      ||
    ################################
    */
    [[nodiscard]] u8 HandleCpuRead( u16 address );
    void             HandleCpuWrite( u16 address, u8 data );

    /*
    ################################
    ||       Internal Reads       ||
    ################################
    */
    [[nodiscard]] u8 Read( u16 addr );
    [[nodiscard]] u8 ReadPatternTable( u16 addr );
    [[nodiscard]] u8 ReadNameTable( u16 addr );

    /*
    ################################
    ||       Internal Writes      ||
    ################################
    */
    void Write( u16 addr, u8 data );
    void WritePatternTable( u16 addr, u8 data );
    void WriteNameTable( u16 addr, u8 data );

    /*
    ################################
    ||         PPU Methods        ||
    ################################
    */
    void DmaTransfer( u8 data );
    u16  ResolveNameTableAddress( u16 addr );
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
    void TriggerNmi();
    void Reset()
    {
        _scanline = 0;
        _cycle = 4;
        _frame = 1;
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
    void EnableJsonTestMode() { _isDisabled = true; }
    void DisableJsonTestMode() { _isDisabled = false; }

  private:
    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
    s16            _scanline = 0;
    u16            _cycle = 4;
    u64            _frame = 1;
    bool           _isRenderingEnabled = false;
    bool           _preventVBlank = false;
    array<u32, 64> _nesPaletteRgbValues{};

    /*
    ################################
    ||        SDL Variables       ||
    ################################
    */
    array<u32, 61440> _frameBuffer{};

    /*
    ################################
    ||       Debug Variables      ||
    ################################
    */
    bool _isDisabled = false;

    /*
    ######################################
    ||  Background Rendering Variables  ||
    ######################################
    */
    u8  _nametableByte = 0x00;
    u8  _attributeByte = 0x00;
    u8  _bgPlane0Byte = 0x00;
    u8  _bgPlane1Byte = 0x00;
    u16 _bgShiftPatternLow = 0x0000;
    u16 _bgShiftPatternHigh = 0x0000;
    u16 _bgShiftAttributeLow = 0x0000;
    u16 _bgShiftAttributeHigh = 0x0000;

    /*
    ################################
    ||         Peripherals        ||
    ################################
    */
    Bus *_bus;

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

    PPUCTRL   _ppuCtrl;          // $2000
    PPUMASK   _ppuMask;          // $2001
    PPUSTATUS _ppuStatus;        // $2002
    u8        _oamAddr = 0x00;   // $2003
    u8        _oamData = 0x00;   // $2004
    u8        _ppuScroll = 0x00; // $2005
    u8        _ppuAddr = 0x00;   // $2006
    u8        _ppuData = 0x00;   // $2007
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
    LoopyRegister _vramAddr;
    LoopyRegister _tempAddr;

    // Internal fine X scroll register
    u8 _fineX = 0x00;

    // Used by _ppuScroll and _ppuAddr for two-write operations
    bool _addrLatch = false;

    // Stores last data written to _ppuData
    u8 _ppuDataBuffer = 0x00;

    /*
    ################################################################
    ||                  Internal Memory Locations                 ||
    ################################################################
    */
    array<u8, 2048> _nameTables{};

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
    array<u8, 0x20> _paletteMemory = {
        0x09, 0x01, 0x00, 0x01, // Palette 0
        0x00, 0x02, 0x02, 0x0D, // Palette 1
        0x08, 0x10, 0x08, 0x24, // Palette 2
        0x00, 0x00, 0x04, 0x2C, // Palette 3
        0x09, 0x01, 0x34, 0x03, // Palette 4
        0x00, 0x04, 0x00, 0x14, // Palette 5
        0x08, 0x3A, 0x00, 0x02, // Palette 6
        0x00, 0x20, 0x2C, 0x08  // Palette 7
    };

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
    array<u8, 256> _oam{};
};

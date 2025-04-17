#pragma once
#include "paths.h"
#include "cpu.h"
#include "global-types.h"
#include "ppu-types.h"
#include "mappers/mapper-base.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>

using namespace std;

class PPU
{
public:
  PPU( Bus *bus );
  Bus *bus;

  /*
  ################################
  ||      Helper Variables      ||
  ################################
  */
  std::array<std::string, 3> systemPalettePaths = { std::string( paths::palettes() ) + "/palette1.pal",
                                                    std::string( paths::palettes() ) + "/palette2.pal",
                                                    std::string( paths::palettes() ) + "/palette3.pal" };

  array<u32, 64> nesPaletteRgbValues{};
  u32            GetMasterPaletteColor( u8 index ) const { return nesPaletteRgbValues.at( index ); }

  bool preventVBlank = false;
  bool nmiReady = false;
  bool failedPaletteRead = false;
  int  systemPaletteIdx = 0;
  int  maxSystemPalettes = 3;

  /*
  ################################
  ||        PPU Variables       ||
  ################################
  */
  u16  scanline = 0;
  void SetScanline( u16 line ) { scanline = line; }

  u16  cycle = 0;
  void SetCycles( u16 cycles ) { cycle = cycles; }

  u64 frame = 1;

  PPUCTRL ppuCtrl; // $2000
  u8      GetPpuCtrl() const { return ppuCtrl.value; }
  u8      GetCtrlNametableX() const { return ppuCtrl.bit.nametableX; }
  u8      GetCtrlNametableY() const { return ppuCtrl.bit.nametableY; }
  u8      GetCtrlIncrementMode() const { return ppuCtrl.bit.vramIncrement; }
  u8      GetCtrlPatternSprite() const { return ppuCtrl.bit.patternSprite; }
  u8      GetCtrlPatternBackground() const { return ppuCtrl.bit.patternBackground; }
  u8      GetCtrlSpriteSize() const { return ppuCtrl.bit.spriteSize; }
  u8      GetCtrlNmiEnable() const { return ppuCtrl.bit.nmiEnable; }

  PPUMASK ppuMask; // $2001
  u8      GetPpuMask() const { return ppuMask.value; }
  u8      GetMaskGrayscale() const { return ppuMask.bit.grayscale; }
  u8      GetMaskRenderBackgroundLeft() const { return ppuMask.bit.renderBackgroundLeft; }
  u8      GetMaskRenderSpritesLeft() const { return ppuMask.bit.renderSpritesLeft; }
  u8      GetMaskRenderBackground() const { return ppuMask.bit.renderBackground; }
  u8      GetMaskRenderSprites() const { return ppuMask.bit.renderSprites; }
  u8      GetMaskEnhanceRed() const { return ppuMask.bit.enhanceRed; }
  u8      GetMaskEnhanceGreen() const { return ppuMask.bit.enhanceGreen; }
  u8      GetMaskEnhanceBlue() const { return ppuMask.bit.enhanceBlue; }
  bool    IsRenderingEnabled() const { return GetMaskRenderBackground() || GetMaskRenderSprites(); }

  PPUSTATUS ppuStatus; // $2002
  u8        GetPpuStatus() const { return ppuStatus.value; }
  u8        GetStatusSpriteOverflow() const { return ppuStatus.bit.spriteOverflow; }
  u8        GetStatusSpriteZeroHit() const { return ppuStatus.bit.spriteZeroHit; }
  u8        GetStatusVblank() const { return ppuStatus.bit.vBlank; }

  u8 oamAddr = 0x00;   // $2003
  u8 oamData = 0x00;   // $2004
  u8 ppuScroll = 0x00; // $2005
  u8 ppuAddr = 0x00;   // $2006
  u8 ppuData = 0x00;   // $2007

  LoopyRegister vramAddr;
  u16           GetVramAddr() const { return vramAddr.value; }
  u8            GetVramCoarseX() const { return vramAddr.bit.coarseX; }
  u8            GetVramCoarseY() const { return vramAddr.bit.coarseY; }
  u8            GetVramNametableX() const { return vramAddr.bit.nametableX; }
  u8            GetVramNametableY() const { return vramAddr.bit.nametableY; }
  u8            GetVramFineY() const { return vramAddr.bit.fineY; }
  u8            GetVramUnused() const { return vramAddr.bit.unused; }

  LoopyRegister tempAddr;
  u16           GetTempAddr() const { return tempAddr.value; }
  u8            GetTempCoarseX() const { return tempAddr.bit.coarseX; }
  u8            GetTempCoarseY() const { return tempAddr.bit.coarseY; }
  u8            GetTempNametableX() const { return tempAddr.bit.nametableX; }
  u8            GetTempNametableY() const { return tempAddr.bit.nametableY; }
  u8            GetTempFineY() const { return tempAddr.bit.fineY; }
  u8            GetTempUnused() const { return tempAddr.bit.unused; }

  u8 fineX = 0x00;
  u8 GetFineX() const { return fineX; }

  bool addrLatch = false;
  bool GetAddrLatch() const { return addrLatch; }

  u8 vramBuffer = 0x00;

  using nametable_t = std::array<u8, 1024>;
  array<nametable_t, 4> nameTables{};

  // u8 nametables[4][1024];
  array<u8, 32> defaultPalette = { 0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D, 0x08, 0x10, 0x08,
                                   0x24, 0x00, 0x00, 0x04, 0x2C, 0x09, 0x01, 0x34, 0x03, 0x00, 0x04,
                                   0x00, 0x14, 0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08 };

  array<u8, 32> paletteMemory = defaultPalette;
  u8            GetPaletteEntry( u8 index ) const { return paletteMemory.at( index ); }
  void          SetPaletteEntry( u8 index, u8 value ) { paletteMemory.at( index ) = value; }

  OAM          oam{};
  SpriteEntry  GetOamEntry( u8 index ) const { return oam.entries.at( index ); }
  SecondaryOAM secondaryOam{};
  SpriteEntry  GetSecondaryOamEntry( u8 index ) const { return secondaryOam.entries.at( index ); }

  /*
  ################################
  ||     Rendering Variables    ||
  ################################
  */
  static constexpr u16 gPrerenderScanline = 261;

  u8  nametableByte = 0x00;
  u8  attributeByte = 0x00;
  u8  bgPattern0Byte = 0x00;
  u8  bgPattern1Byte = 0x00;
  u16 bgPatternShiftLow = 0x0000;  // holds bitplane 0 (low bits) of tile pixels
  u16 bgPatternShiftHigh = 0x0000; // holds bitplane 1 (high bits) of tile pixels
  u16 bgAttributeShiftLow = 0x00;
  u16 bgAttributeShiftHigh = 0x00;

  std::array<u8, 8> spriteShiftLow{};  // low bitplane pattern data for each sprite
  std::array<u8, 8> spriteShiftHigh{}; // high bitplane pattern data for each sprite

  u8 spritePattern0Byte = 0x00;
  u8 spritePattern1Byte = 0x00;

  // Sprite helpers
  bool bSpriteZeroHitPossible = false;
  bool bSprite0Appeared = false;
  u8   spriteCount = 0;
  u8   nOamEntry = 0;

  // SDL callbacks
  std::function<void( const u32 * )> onFrameReady = nullptr;

  /*
  ################################
  ||       Debug Variables      ||
  ################################
  */
  bool isDisabled = false;
  void EnableJsonTestMode() { isDisabled = true; }
  void DisableJsonTestMode() { isDisabled = false; }

  /*
  ################################
  ||        SDL Variables       ||
  ################################
  */
  static constexpr int    gBufferSize = 61440;
  array<u32, gBufferSize> frameBuffer{};
  array<u32, gBufferSize> GetFrameBuffer() const { return frameBuffer; }

  void ClearFrameBuffer() { frameBuffer.fill( 0x00000000 ); }

  /*
  ################################
  ||     Method Definitions    ||
  ################################
  */
  MirrorMode GetMirrorMode() const;
  u8         CpuRead( u16 address, bool debugMode = false );
  void       CpuWrite( u16 address, u8 data );
  u8         ReadVram( u16 addr );
  void       WriteVram( u16 addr, u8 data );
  u16        ResolveNameTableAddress( u16 addr, int testMirrorMode = -1 ) const;
  void       Tick();
  void       VBlank();

  /*
  ################################
  ||            Utils           ||
  ################################
  */
  bool InCycle( int left, int right ) const { return left <= cycle && cycle <= right; }
  bool InScanline( int left, int right ) const { return left <= scanline && scanline <= right; }

  /*
  ################################################################
  ||                                                            ||
  ||                    Inline Helper Methods                   ||
  ||                                                            ||
  ################################################################
  */

  void LoadBgShifters()
  {
    /* @brief Loads the next background tile information (lower 8 bits) into the background shifters
     */
    bgPatternShiftLow = ( bgPatternShiftLow & 0xFF00 ) | bgPattern0Byte;
    bgPatternShiftHigh = ( bgPatternShiftHigh & 0xFF00 ) | bgPattern1Byte;
    u8 const attrMaskLow = ( attributeByte & 0b01 ) ? 0xFF : 0x00;
    u8 const attrMaskHigh = ( attributeByte & 0b10 ) ? 0xFF : 0x00;
    bgAttributeShiftLow = ( bgAttributeShiftLow & 0xFF00 ) | attrMaskLow;
    bgAttributeShiftHigh = ( bgAttributeShiftHigh & 0xFF00 ) | attrMaskHigh;
  }

  void UpdateShifters()
  {
    if ( ppuMask.bit.renderBackground ) {
      bgPatternShiftLow <<= 1;
      bgPatternShiftHigh <<= 1;
      bgAttributeShiftLow <<= 1;
      bgAttributeShiftHigh <<= 1;
    }
    if ( InCycle( 1, 256 ) ) {
      if ( ppuMask.bit.renderSprites ) {
        for ( int i = 0; i < spriteCount; i++ ) {
          if ( secondaryOam.entries.at( i ).x > 0 ) {
            secondaryOam.entries.at( i ).x--;
          } else {
            spriteShiftLow.at( i ) <<= 1;
            spriteShiftHigh.at( i ) <<= 1;
          }
        }
      }
    }
  }

  void OddFrameSkip()
  {
    bool const isOddFrame = frame & 0x01;
    if ( isOddFrame && scanline == 0 && cycle == 0 ) {
      cycle = 1;
    }
  }

  void PrerenderScanline()
  {
    if ( scanline != gPrerenderScanline ) {
      return;
    }

    // Clear status to get ready for next frame
    if ( cycle == 1 ) {
      ppuStatus.bit.vBlank = 0;
      ppuStatus.bit.spriteZeroHit = 0;
      ppuStatus.bit.spriteOverflow = 0;
    }

    VisibleScanline();

    // Transfer from temp to vramAddr on cycles 280-304
    if ( InCycle( 280, 304 ) ) {
      TransferAddressY();
    }
  }

  void VisibleScanline()
  {
    if ( InCycle( 1, 256 ) ) {
      FetchBgTileData();
    }

    if ( cycle == 257 ) {
      LoadBgShifters();
      TransferAddressX();
      SpriteEval();
    }

    // Cycles 321-336 will fetch the first two tiles for the next scanline
    if ( InCycle( 321, 336 ) ) {
      FetchBgTileData();
    }

    // Unused fetches
    if ( cycle == 338 || cycle == 340 ) {
      FetchNametableByte();
    }
    // Fetch sprite data
    if ( cycle == 340 ) {
      FetchSpriteData();
    }
  }

  void FetchBgTileData()
  {

    UpdateShifters();
    // Background fetches (cycles 1–256 and 321–336)
    switch ( ( cycle - 1 ) & 0x07 ) {

      case 0:
        LoadBgShifters();
        FetchNametableByte();
        break;
      case 2:
        FetchAttributeByte();
        break;
      case 4:
        FetchBgPattern0Byte();
        break;
      case 6:
        FetchBgPattern1Byte();
        break;
      case 7:
        IncrementCoarseX();
        if ( cycle == 256 )
          IncrementCoarseY();
        break;
      default:
    }
  }

  void SpriteEval()
  {
    if ( !IsRenderingEnabled() || cycle != 257 )
      return;

    std::memset( secondaryOam.data.data(), 0xFF, secondaryOam.data.size() );
    spriteCount = 0;
    for ( u8 i = 0; i < 8; i++ ) {
      spriteShiftLow[i] = 0;
      spriteShiftHigh[i] = 0;
    }
    nOamEntry = 0;
    bSpriteZeroHitPossible = false;

    while ( nOamEntry < 64 && spriteCount < 9 ) {
      auto const sprite = oam.entries.at( nOamEntry );
      bool       const spriteInRange = IsSpriteInRange( scanline, sprite.y, (bool) ppuCtrl.bit.spriteSize );

      if ( spriteInRange ) {
        if ( spriteCount < 8 ) {
          if ( nOamEntry == 0 ) {
            bSpriteZeroHitPossible = true;
          }
          secondaryOam.entries.at( spriteCount ) = sprite;
          spriteCount++;
        }
      }
      nOamEntry++;
    }

    ppuStatus.bit.spriteOverflow = ( spriteCount > 8 );
  }

  static bool IsSpriteInRange( int scanline, int y, bool isLarge )
  {
    return scanline >= y && scanline < y + ( isLarge ? 16 : 8 );
  }

  // Copies from primary to secondary oam
  void CopyToSecondaryOam() {}

  void FetchSpriteData()
  {
    for ( u8 i = 0; i < spriteCount; i++ ) {
      SpriteEntry const sprite = secondaryOam.entries[i];

      u8   spritePattern0Byte = 0;
      u8   spritePattern1Byte = 0;
      u16  spritePattern0Addr = 0;
      u16  spritePattern1Addr = 0;
      bool const isLarge = ppuCtrl.bit.spriteSize;

      u16 baseAddr = ppuCtrl.bit.patternSprite << 12; // 0x0000 or 0x1000 depending on ppuCtrl (for 8x8 sprites)
      u8  tileIndex = sprite.tileIndex;               // 0 - 16
      u8  rowOffset = scanline - sprite.y;            // 0 - 7

      // 8x8
      if ( !isLarge ) {
        // Not vertically flipped
        if ( !sprite.attribute.bit.flipV ) {
          spritePattern0Addr = baseAddr | tileIndex << 4 | rowOffset;
        }
        // vertically flipped
        else {
          spritePattern0Addr = baseAddr | tileIndex << 4 | ( 7 - rowOffset );
        }
      }
      // 8x16
      else {
        baseAddr = ( sprite.tileIndex & 0x1 ) << 12; // offset depends on first bit of the tile index for 8x16 sprites
        tileIndex = sprite.tileIndex & ~0x1;         // other bits of tile index determine the tileIndex
        rowOffset = ( scanline - sprite.y ) & 0x07;

        // Not vertically flipped
        if ( !sprite.attribute.bit.flipV ) {
          // Top half
          if ( scanline - sprite.y < 8 ) {
            spritePattern0Addr = baseAddr | ( tileIndex << 4 ) | rowOffset;
          }
          // Bottom half
          else {
            spritePattern0Addr = baseAddr | ( ( tileIndex + 1 ) << 4 ) | rowOffset;
          }
        }
        // vertically flipped
        else {
          // Top half
          if ( scanline - sprite.y < 8 ) {
            spritePattern0Addr = baseAddr | ( ( tileIndex + 1 ) << 4 ) | ( 7 - rowOffset );
          }
          // bottom half
          else {
            spritePattern0Addr = baseAddr | ( tileIndex << 4 ) | ( 7 - rowOffset );
          }
        }
      }

      spritePattern1Addr = spritePattern0Addr + 8;
      spritePattern0Byte = ReadVram( spritePattern0Addr );
      spritePattern1Byte = ReadVram( spritePattern1Addr );

      auto flipbyte = []( uint8_t b ) {
        b = ( b & 0xF0 ) >> 4 | ( b & 0x0F ) << 4;
        b = ( b & 0xCC ) >> 2 | ( b & 0x33 ) << 2;
        b = ( b & 0xAA ) >> 1 | ( b & 0x55 ) << 1;
        return b;
      };

      if ( sprite.attribute.bit.flipH ) {
        spritePattern0Byte = flipbyte( spritePattern0Byte );
        spritePattern1Byte = flipbyte( spritePattern1Byte );
      }

      spriteShiftLow.at( i ) = spritePattern0Byte;
      spriteShiftHigh.at( i ) = spritePattern1Byte;
    }
  }

  u32 GetOutputPixel()
  {
    u8 bgPixel = 0;
    u8 bgPalette = 0;
    FetchBackgroundPixel( bgPixel, bgPalette );

    u8 fgPixel = 0;
    u8 fgPalette = 0;
    u8 fgPriority = 0;
    FetchForegroundPixel( fgPixel, fgPalette, fgPriority );

    u8 outPixel = 0x00;
    u8 outPalette = 0x00;

    // Both background and foreground are transparent.
    if ( bgPixel == 0 && fgPixel == 0 ) {
      outPixel = 0;
      outPalette = 0;
    }

    // Background transparent, foreground visible.
    else if ( bgPixel == 0 && fgPixel > 0 ) {
      outPixel = fgPixel;
      outPalette = fgPalette;
    }

    // Background visible, foreground transparent.
    else if ( bgPixel > 0 && fgPixel == 0 ) {
      outPixel = bgPixel;
      outPalette = bgPalette;
    }

    // Both are non-transparent. Sprite priority determines which one is visible.
    else if ( bgPixel > 0 && fgPixel > 0 ) {
      if ( fgPriority ) {
        outPixel = fgPixel;
        outPalette = fgPalette;
      } else {
        outPixel = bgPixel;
        outPalette = bgPalette;
      }
    }

    // Sprite Zero Hit detection
    if ( bSpriteZeroHitPossible && bSprite0Appeared ) {
      if ( ppuMask.bit.renderBackground & ppuMask.bit.renderSprites ) {
        if ( ~( ppuMask.bit.renderBackgroundLeft | ppuMask.bit.renderSpritesLeft ) ) {
          if ( cycle >= 9 && cycle < 256 ) {
            ppuStatus.bit.spriteZeroHit = 1;
          }
        } else {
          if ( cycle >= 1 && cycle < 256 ) {
            ppuStatus.bit.spriteZeroHit = 1;
          }
        }
      }
    }

    // Write final color to framebuffer
    u16 const paletteAddr = 0x3F00 + ( outPalette << 2 ) + outPixel;
    u8 const  paletteIdx = ReadVram( paletteAddr ) & 0x3F;
    u32 const rgbColor = nesPaletteRgbValues.at( paletteIdx );
    return rgbColor;

    return 0;
  }

  void FetchBackgroundPixel( u8 &pixel, u8 &palette ) const
  {
    if ( ppuMask.bit.renderBackground && ( ppuMask.bit.renderBackgroundLeft || cycle >= 9 ) ) {
      // Fine X scrolling: determine the bit offset in our shifters.
      u16 const bitMux = 0x8000 >> fineX;
      // Extract the pixel bits from background shifters.
      u8 const p0Pixel = ( bgPatternShiftLow & bitMux ) ? 1 : 0;
      u8 const p1Pixel = ( bgPatternShiftHigh & bitMux ) ? 1 : 0;
      pixel = ( p1Pixel << 1 ) | p0Pixel;

      // Extract the palette bits from attribute shifters.
      u8 const pal0 = ( bgAttributeShiftLow & bitMux ) ? 1 : 0;
      u8 const pal1 = ( bgAttributeShiftHigh & bitMux ) ? 1 : 0;
      palette = ( pal1 << 1 ) | pal0;
    }
  }

  void FetchForegroundPixel( u8 &pixel, u8 &palette, u8 &priority )
  {
    if ( ppuMask.bit.renderSprites ) {
      bSprite0Appeared = false;

      for ( u8 i = 0; i < spriteCount; i++ ) {
        SpriteEntry const sprite = secondaryOam.entries[i];

        // Only consider sprites whose shift registers are active (x==0).
        if ( sprite.x == 0 ) {
          u8 const plane0Bit = ( spriteShiftLow[i] & 0x80 ) > 0;
          u8 const plane1Bit = ( spriteShiftHigh[i] & 0x80 ) > 0;
          pixel = ( plane1Bit << 1 ) | plane0Bit;
          palette = ( sprite.attribute.bit.palette ) + 0x04;
          priority = sprite.attribute.bit.priority == 0;

          // As soon as we find a nontransparent sprite pixel, use it.
          if ( pixel != 0 ) {
            if ( i == 0 ) {
              bSprite0Appeared = true;
            }
            break;
          }
        }
      }
    }
  }

  void FetchAttributeByte()
  {
    u16 const nametableSelect = vramAddr.value & 0x0C00;
    u8 const  attrX = vramAddr.bit.coarseX >> 2;
    u8 const  attrY = ( vramAddr.bit.coarseY >> 2 ) << 3;
    u16 const attrAddr = 0x23C0 | nametableSelect | attrY | attrX;
    attributeByte = ReadVram( attrAddr );
    if ( vramAddr.bit.coarseY & 0x02 )
      attributeByte >>= 4;
    if ( vramAddr.bit.coarseX & 0x02 )
      attributeByte >>= 2;
    attributeByte &= 0x03;
  }

  void FetchNametableByte() { nametableByte = ReadVram( 0x2000 | ( vramAddr.value & 0x0FFF ) ); }

  void FetchBgPattern0Byte()
  {
    u16 const bgPatternOffset = ppuCtrl.bit.patternBackground << 12;
    u16 const tileBase = nametableByte << 4;
    u16 const rowOffset = vramAddr.bit.fineY;
    bgPattern0Byte = ReadVram( bgPatternOffset | tileBase | rowOffset );
  }

  void FetchBgPattern1Byte()
  {
    u16 const bgPatternOffset = ppuCtrl.bit.patternBackground << 12;
    u16 const tileBase = nametableByte << 4;
    u16 const rowOffset = vramAddr.bit.fineY;
    bgPattern1Byte = ReadVram( ( bgPatternOffset | tileBase | rowOffset ) + 8 );
  }

  void IncrementCoarseX()
  {
    if ( !IsRenderingEnabled() ) {
      return;
    }

    if ( vramAddr.bit.coarseX == 31 ) {
      vramAddr.bit.coarseX = 0;
      vramAddr.bit.nametableX = ~vramAddr.bit.nametableX;
    } else {
      vramAddr.bit.coarseX++;
    }
  }

  void IncrementCoarseY()
  {
    if ( !IsRenderingEnabled() ) {
      return;
    }
    if ( vramAddr.bit.fineY < 7 ) {
      vramAddr.bit.fineY++;
    } else {
      vramAddr.bit.fineY = 0;
      if ( vramAddr.bit.coarseY == 29 ) {
        vramAddr.bit.coarseY = 0;
        vramAddr.bit.nametableY = ~vramAddr.bit.nametableY;
      } else if ( vramAddr.bit.coarseY == 31 ) {
        vramAddr.bit.coarseY = 0;
      } else {
        vramAddr.bit.coarseY++;
      }
    }
  }

  void UpdateFrameBuffer( int debugValue = -1 )
  {
    if ( InScanline( 0, 239 ) && InCycle( 1, 256 ) ) {
      u16 const bufferIdx = ( scanline * 256 ) + ( cycle - 1 );
      if ( debugValue > -1 ) {
        frameBuffer.at( bufferIdx ) = debugValue;
      } else {
        frameBuffer.at( bufferIdx ) = GetOutputPixel();
      }
    }
  }

  void RenderFrameBuffer()
  {
    if ( onFrameReady ) {
      onFrameReady( frameBuffer.data() );
    }
  }

  void TransferAddressX()
  {
    if ( IsRenderingEnabled() ) {
      vramAddr.bit.nametableX = tempAddr.bit.nametableX;
      vramAddr.bit.coarseX = tempAddr.bit.coarseX;
    }
  }

  void TransferAddressY()
  {
    if ( IsRenderingEnabled() ) {
      vramAddr.bit.fineY = tempAddr.bit.fineY;
      vramAddr.bit.nametableY = tempAddr.bit.nametableY;
      vramAddr.bit.coarseY = tempAddr.bit.coarseY;
    }
  }

  void Reset()
  {
    scanline = 0;
    cycle = 0;
    frame = 1;
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
    vramBuffer = 0x00;
    vramAddr.value = 0x0000;
    tempAddr.value = 0x0000;
    fineX = 0x00;
    for ( auto &table : nameTables ) {
      table.fill( 0x00 );
    }
    paletteMemory = defaultPalette;
    ClearFrameBuffer();
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
    nesPaletteRgbValues = ReadPalette( palettePath );
  }

  u8 GetPpuPaletteValue( u8 index ) { return paletteMemory.at( index ); }

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

  // Get pattern table data, used in debugging (frontend/ui/pattern-tables.h)
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
        u8 const plane0Byte = ReadVram( tileAddr + row );
        u8 const plane1Byte = ReadVram( tileAddr + row + 8 );
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

  // Used in frontend/ui/sprites.h
  std::array<u32, 4096> GetOamSpriteData()
  {
    std::array<u32, 4096> buffer{};
    u16                   const baseAddr = ppuCtrl.bit.patternSprite ? 0x1000 : 0x0000;

    for ( int tileY = 0; tileY < 8; tileY++ ) {
      for ( int tileX = 0; tileX < 8; tileX++ ) {
        int const spriteIndex = ( tileY * 8 ) + tileX;
        auto      entry = GetOamEntry( spriteIndex );
        u16 const tileAddr = baseAddr | ( entry.tileIndex << 4 );

        for ( int row = 0; row < 8; row++ ) {
          u8 const plane0Byte = ReadVram( tileAddr + row );
          u8 const plane1Byte = ReadVram( tileAddr + row + 8 );

          for ( int bit = 7; bit >= 0; bit-- ) {
            // Calculate the pixel's position in the 64x64 output buffer.
            int const localX = 7 - bit;
            int const globalX = ( tileX * 8 ) + localX;
            int const globalY = ( tileY * 8 ) + row;
            int const bufferIdx = ( globalY * 64 ) + globalX;

            // Calculate pixel color
            u8 const  plane0Bit = ( plane0Byte >> bit ) & 0x01;
            u8 const  plane1Bit = ( plane1Byte >> bit ) & 0x01;
            u8 const  colorOffset = ( plane1Bit << 1 ) | plane0Bit;
            u8 const  paletteBase = 16 + ( entry.attribute.bit.palette * 4 );
            u16 const vramAddr = 0x3F00 + paletteBase + colorOffset;
            u8 const  paletteIdx = ReadVram( vramAddr );

            // Output
            buffer.at( bufferIdx ) = GetMasterPaletteColor( paletteIdx );
          }
        }
      }
    }
    return buffer;
  }

  // Nametable data, used in debugging (frontend/ui/nametables.h)
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
      u8 const tileIndex = ReadVram( vramAddr );

      // Combining the two gives the pattern table address
      u16 const tileAddr = patternTableBaseAddr + ( tileIndex * 16 );

      // Grab the attribute byte, which covers a 32x32 area
      u8 const  attrX = tileX / 4;
      u8 const  attrY = tileY / 4;
      u16 const attrAddr = attrBase + ( attrY * 8 ) + attrX;
      u8 const  attributeByte = ReadVram( attrAddr );

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
        u8 const plane0Byte = ReadVram( tileAddr + pixelRow );
        u8 const plane1Byte = ReadVram( tileAddr + pixelRow + 8 );
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

  static array<u32, 64> ReadPalette( const string &filename )
  {
    array<u32, 64> nesPalette{};

    std::ifstream file( filename, std::ios::binary );
    if ( !file ) {
      std::cerr << "PPU::ReadPalette: Failed to open palette file: " << filename << '\n';
      throw std::runtime_error( "Failed to open palette file" );
    }

    file.seekg( 0, std::ios::end );
    streamsize const fileSize = file.tellg();
    if ( fileSize != 192 ) {
      std::cerr << "Invalid palette file size: " << fileSize << '\n';
      throw std::runtime_error( "Invalid palette file size" );
    }

    file.seekg( 0, std::ios::beg );

    char buffer[192]; // NOLINT
    if ( !file.read( buffer, 192 ) ) {
      std::cerr << "Failed to read palette file: " << filename << '\n';
      throw std::runtime_error( "Failed to read palette file" );
    }

    // Convert to 32-bit RGBA (SDL_PIXELFORMAT_RGBA32)
    for ( int i = 0; i < 64; ++i ) {
      u8 const red = buffer[( i * 3 ) + 0];
      u8 const green = buffer[( i * 3 ) + 1];
      u8 const blue = buffer[( i * 3 ) + 2];
      u8 const alpha = 0xFF;
      nesPalette[i] = ( alpha << 24 ) | ( blue << 16 ) | ( green << 8 ) | red;
    }

    return nesPalette;
  }
};

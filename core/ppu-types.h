#pragma once
#include <array>
#include "global-types.h"

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

  template <class Archive> void serialize( Archive &ar ) { ar( value ); } // NOLINT
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

  template <class Archive> void serialize( Archive &ar ) { ar( value ); } // NOLINT
};
union PPUSTATUS {
  struct {
    u8 unused : 5;
    u8 spriteOverflow : 1;
    u8 spriteZeroHit : 1;
    u8 vBlank : 1;
  } bit;
  u8 value = 0x00;

  template <class Archive> void serialize( Archive &ar ) { ar( value ); } // NOLINT
};

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

  template <class Archive> void serialize( Archive &ar ) { ar( value ); } // NOLINT
};

union SpriteAttribute {
  struct {
    u8 palette : 2;
    u8 unused : 3;
    u8 priority : 1;
    u8 flipH : 1;
    u8 flipV : 1;
  } bit;
  u8 value = 0x00;

  template <class Archive> void serialize( Archive &ar ) { ar( value ); } // NOLINT
};

struct SpriteEntry {
  u8              y;
  u8              tileIndex;
  SpriteAttribute attribute;
  u8              x;

  template <class Archive> void serialize( Archive &ar ) { ar( y, tileIndex, attribute, x ); } // NOLINT
};
union OAM {
  std::array<u8, 256>         data{};
  std::array<SpriteEntry, 64> entries;

  template <class Archive> void serialize( Archive &ar ) { ar( data ); } // NOLINT
};
union SecondaryOAM {
  std::array<u8, 32>         data{};
  std::array<SpriteEntry, 8> entries;

  template <class Archive> void serialize( Archive &ar ) { ar( data ); } // NOLINT
};

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
        u8 vBlank : 1;
    } bit;
    u8 value = 0x00;
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
};

struct SpriteEntry {
    u8 y;
    u8 tileIndex;
    u8 attribute;
    u8 x;
};
union OAM {
    std::array<u8, 256>         data;
    std::array<SpriteEntry, 64> entries;
};
union SecondaryOAM {
    std::array<u8, 32>         data;
    std::array<SpriteEntry, 8> entries;
};

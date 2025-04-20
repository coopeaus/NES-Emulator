#pragma once
#include "global-types.h"

union APUSTATUS {
  struct {
    u8 unused : 5;
    u8 spriteOverflow : 1;
    u8 spriteZeroHit : 1;
    u8 vBlank : 1;
  } bit;
  u8 value = 0x00;
};

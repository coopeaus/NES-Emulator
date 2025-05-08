// Implementation adopted from javidx9 NES series: https://youtu.be/AAcEk5pWXPY?si=HHJ8Tqm3iVRwWf9O

#include "mapper4.h"
#include "mappers/mapper-base.h"
#include "utils.h"
using utils::between;

u32 Mapper4::MapCpuAddr( u16 addr )
{
  if ( between( addr, 0x6000, 0x7FFF ) ) {
    return addr & 0x1FFF;
  }
  if ( between( addr, 0x8000, 0x9FFF ) ) {
    return pPrgBank[0] + ( addr & 0x1FFF );
  }
  if ( between( addr, 0xA000, 0xBFFF ) ) {
    return pPrgBank[1] + ( addr & 0x1FFF );
  }
  if ( between( addr, 0xC000, 0xDFFF ) ) {
    return pPrgBank[2] + ( addr & 0x1FFF );
  }
  if ( between( addr, 0xE000, 0xFFFF ) ) {
    return pPrgBank[3] + ( addr & 0x1FFF );
  }
  return 0;
}

u32 Mapper4::MapPpuAddr( u16 addr )
{
  if ( between( addr, 0x0000, 0x03FF ) ) {
    return pChrBank[0] + ( addr & 0x03FF );
  }
  if ( between( addr, 0x0400, 0x07FF ) ) {
    return pChrBank[1] + ( addr & 0x03FF );
  }
  if ( between( addr, 0x0800, 0x0BFF ) ) {
    return pChrBank[2] + ( addr & 0x03FF );
  }
  if ( between( addr, 0x0C00, 0x0FFF ) ) {
    return pChrBank[3] + ( addr & 0x03FF );
  }
  if ( between( addr, 0x1000, 0x13FF ) ) {
    return pChrBank[4] + ( addr & 0x03FF );
  }
  if ( between( addr, 0x1400, 0x17FF ) ) {
    return pChrBank[5] + ( addr & 0x03FF );
  }
  if ( between( addr, 0x1800, 0x1BFF ) ) {
    return pChrBank[6] + ( addr & 0x03FF );
  }
  if ( between( addr, 0x1C00, 0x1FFF ) ) {
    return pChrBank[7] + ( addr & 0x03FF );
  }
  return 0;
}

void Mapper4::HandleCPUWrite( u16 addr, u8 data )
{
  if ( addr < 0x8000 )
    return;
  if ( between( addr, 0x8000, 0x9FFF ) ) {
    // even addr, bank select
    if ( ( addr & 1 ) == 0 ) {
      nTargetRegister = data & 0x07;
      bPrgBankMode = data & 0x40;
      bChrInversion = data & 0x80;
    } else {
      // Update target register
      pRegister[nTargetRegister] = data;

      // Update pointer table
      if ( bChrInversion ) {
        pChrBank[0] = pRegister[2] * 0x0400;
        pChrBank[1] = pRegister[3] * 0x0400;
        pChrBank[2] = pRegister[4] * 0x0400;
        pChrBank[3] = pRegister[5] * 0x0400;
        pChrBank[4] = ( pRegister[0] & 0xFE ) * 0x0400;
        pChrBank[5] = pRegister[0] * 0x0400 + 0x0400;
        pChrBank[6] = ( pRegister[1] & 0xFE ) * 0x0400;
        pChrBank[7] = pRegister[1] * 0x0400 + 0x0400;
      } else {
        pChrBank[0] = ( pRegister[0] & 0xFE ) * 0x0400;
        pChrBank[1] = pRegister[0] * 0x0400 + 0x0400;
        pChrBank[2] = ( pRegister[1] & 0xFE ) * 0x0400;
        pChrBank[3] = pRegister[1] * 0x0400 + 0x0400;
        pChrBank[4] = pRegister[2] * 0x0400;
        pChrBank[5] = pRegister[3] * 0x0400;
        pChrBank[6] = pRegister[4] * 0x0400;
        pChrBank[7] = pRegister[5] * 0x0400;
      }

      if ( bPrgBankMode ) {
        pPrgBank[2] = ( pRegister[6] & 0x3F ) * 0x2000;
        pPrgBank[0] = ( GetPrgBankCount() * 2 - 2 ) * 0x2000;
      } else {
        pPrgBank[0] = ( pRegister[6] & 0x3F ) * 0x2000;
        pPrgBank[2] = ( GetPrgBankCount() * 2 - 2 ) * 0x2000;
      }

      pPrgBank[1] = ( pRegister[7] & 0x3F ) * 0x2000;
      pPrgBank[3] = ( GetPrgBankCount() * 2 - 1 ) * 0x2000;
    }
    return;
  }

  if ( between( addr, 0xA000, 0xBFFF ) ) {
    if ( ( addr & 1 ) == 0 ) {
      if ( data & 1 ) {
        mirroring = MirrorMode::Horizontal;
      } else {
        mirroring = MirrorMode::Vertical;
      }
    }
    return;
  }
  if ( between( addr, 0xC000, 0xDFFF ) ) {
    if ( ( addr & 1 ) == 0 ) {
      nIrqReload = data;
    } else {
      nIrqCounter = 0x0000;
    }
    return;
  }
  if ( between( addr, 0xE000, 0xFFFF ) ) {
    if ( ( addr & 1 ) == 0 ) {
      bIrqEnabled = false;
      bIsIrqRequested = false;
    } else {
      bIrqEnabled = true;
    }
    return;
  }
}

void Mapper4::Reset()
{
  nTargetRegister = 0x00;
  bPrgBankMode = false;
  bChrInversion = false;
  mirroring = MirrorMode::Horizontal;

  bIsIrqRequested = false;
  bIrqEnabled = false;
  nIrqCounter = 0x0000;
  nIrqReload = 0x0000;

  for ( int i = 0; i < 4; i++ )
    pPrgBank[i] = 0;
  for ( int i = 0; i < 8; i++ ) {
    pChrBank[i] = 0;
    pRegister[i] = 0;
  }

  pPrgBank[0] = 0 * 0x2000;
  pPrgBank[1] = 1 * 0x2000;
  pPrgBank[2] = ( GetPrgBankCount() * 2 - 2 ) * 0x2000;
  pPrgBank[3] = ( GetPrgBankCount() * 2 - 1 ) * 0x2000;
}

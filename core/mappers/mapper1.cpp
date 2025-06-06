#include "mappers/mapper1.h"
#include "global-types.h"
#include "mappers/mapper-base.h"
#include <stdexcept>

MirrorMode Mapper1::GetMirrorMode()
{
  return mirroring;
}

/*
################################
||                            ||
||          CPU Read          ||
||                            ||
################################
*/
[[nodiscard]] u32 Mapper1::MapCpuAddr( u16 address )
{
  /**
   * @details
   * PRG bank selectors point to individual 16 KiB banks within the PRG ROM.
   * The total number of banks is derived from the 4th byte of the NES ROM header,
   * which specifies the size of the PRG ROM in 16 KiB units. For example, if the
   * cartridge provides 4 PRG banks (64 KiB total), the selectors operate as follows:
   *
   * - **32 KiB Mode** (determined by the control register):
   *   - `_prg_bank_32 = 0`:
   *     - Bank 0 (16 KiB) is mapped to `0x8000–0xBFFF`.
   *     - Bank 1 (16 KiB) is mapped to `0xC000–0xFFFF`.
   *   - `_prg_bank_32 = 1`:
   *     - Bank 2 (16 KiB) is mapped to `0x8000–0xBFFF`.
   *     - Bank 3 (16 KiB) is mapped to `0xC000–0xFFFF`.
   *
   * - **16 KiB Mode** (determined by the control register):
   *   - `_prg_bank_16_lo` selects the bank for `0x8000–0xBFFF`.
   *   - `_prg_bank_16_hi` selects the bank for `0xC000–0xFFFF`.
   *
   *   Example Configurations:
   *   - `_prg_bank_16_lo = 0`, `_prg_bank_16_hi = 3`:
   *     - Bank 0 (16 KiB) is mapped to `0x8000–0xBFFF`.
   *     - Bank 3 (16 KiB) is mapped to `0xC000–0xFFFF`.
   *   - `_prg_bank_16_lo = 2`, `_prg_bank_16_hi = 3`:
   *     - Bank 2 (16 KiB) is mapped to `0x8000–0xBFFF`.
   *     - Bank 3 (16 KiB) is mapped to `0xC000–0xFFFF`.
   *
   * In 16 KiB mode, `_prg_bank_16_lo` and `_prg_bank_16_hi` independently select
   * banks for their respective address ranges, allowing finer-grained control.
   *
   * Mapper 1 uses these selectors dynamically, based on writes to the mapper's
   * internal registers, to swap PRG ROM banks as needed during gameplay.
   */

  // 16 KiB mode
  if ( ( controlRegister & 0b00001000 ) != 0 ) {
    if ( address >= 0x8000 && address <= 0xBFFF ) {
      // Swap out the lower 16 KiB bank.
      u32 const bankOffset = prgBank16Lo * 0x4000;
      return bankOffset + ( address & 0x3FFF );
    }
    if ( address >= 0xC000 && address <= 0xFFFF ) {
      // Swap out the upper 16 KiB bank.

      u32 const bankOffset = prgBank16Hi * 0x4000;
      return bankOffset + ( address & 0x3FFF );
    }
  }

  // 32 KiB mode
  u32 const bankOffset = prgBank32 * 0x8000;
  return bankOffset + ( address & 0x7FFF );
}

/*
################################
||                            ||
||          CPU Write         ||
||                            ||
################################
*/
void Mapper1::HandleCPUWrite( u16 address, u8 data )
{
  /** @brief Handle writes to signal bank swapping
   * @details
   * ROM is normally read-only, but mapper 1 (and other mappers)
   * use writes to change the bank selection in the cartridge.
   */

  // Reset state when data with the MSB set is written
  if ( data & 0x80 ) {
    shiftRegister = 0x10;
    writeCount = 0;
    controlRegister |= 0x0C;
    return;
  }

  // Serial loading mechanism
  /* The Mapper 1 serial loading mechanism accumulates 5 writes.
   . Each write shifts the register to the right and inserts the
   * least significant bit (LSB) of the data byte into the most significant bit (MSB)
   * position (bit 4).
   *
   * After 5 writes, the accumulated 5-bit value in `_shift_register` is applied to one
   * of the mapper's registers. The target register is determined by examining bits 13
   * and 14 of the write address. Additional information is encoded in the address, but only
   * for the 5th write:
   *
   *   - 0x8000 – 0x9FFF: Configures mirroring mode, PRG banking mode, and CHR banking mode.
   *   - 0xA000 – 0xBFFF: Selects the lower CHR bank.
   *   - 0xC000 – 0xDFFF: Selects the upper CHR bank (in 4 KB mode).
   *   - 0xE000 – 0xFFFF: Selects the PRG bank(s) to map to the CPU address space.
   *
   * Once the 5-bit value is applied to the selected register, the shift register and
   * the write count are reset to prepare for the next sequence.
   */
  shiftRegister >>= 1;
  shiftRegister |= ( data & 0b00000001 ) << 4;
  writeCount++;
  if ( !( writeCount == 5 ) ) {
    return;
  }

  // On the fifth write, target register is derived from
  // byte 13 and 14 of the address
  u8 const targetRegister = ( address >> 13 ) & 0b00000011;

  if ( targetRegister == 0 ) // 0x8000-0x9FFF
  {
    // Set control register to the first 5 bits of the shift register
    controlRegister = shiftRegister & 0b00011111;

    // the lower 2 bits of the control register set the mirroring mode
    switch ( controlRegister & 0b00000011 ) {
      case 0x00: mirroring = MirrorMode::SingleLower; break;
      case 0x01: mirroring = MirrorMode::SingleUpper; break;
      case 0x02: mirroring = MirrorMode::Vertical; break;
      case 0x03: mirroring = MirrorMode::Horizontal; break;
      default  : throw std::runtime_error( "Invalid mirroring mode" );
    }
  }

  else if ( targetRegister == 1 ) // 0xA000-0xBFFF
  {
    // Determine CHR banking mode (4 KiB or 8 KiB) based on bit 4 of the control register
    bool const is4KbChrMode = ( controlRegister & 0b00010000 ) != 0;

    if ( is4KbChrMode ) {
      // Set lower 4 KB CHR bank

      // In 4 KiB mode, the shift register IS the bank index. The shift register is 5 bits
      // wide, and all 5 bits are used to set the bank index directly. Masking ensures only
      // the lower 5 bits are considered, as `_shift_register` is declared as a u8, and the
      // upper 3 bits are always 0. The `bank_index` determines which 4 KiB CHR bank is mapped
      // to the PPU address space (0x0000–0x0FFF for lower 4 KiB or 0x1000–0x1FFF for upper 4
      // KiB).
      u8 const bankIndex = shiftRegister & 0b00011111; // Indeces: 0 - 31
      chrBank4Lo = bankIndex;
    } else {
      // Set 8 KB CHR bank
      // In 8 KiB, all indeces must start on an even number, other wise the 8 KiB bank will
      // be misaligned. Hence, the mask captures the shift register but clears out the 1 that
      // would have otherwise made the index odd
      u8 const bankIndex = shiftRegister & 0b00011110; // Indeces 0 - 30, even numbers only
      chrBank8 = bankIndex;
    }
  } else if ( targetRegister == 2 ) // 0xC000-0xDFFF
  {
    bool const is4KbChrMode = ( controlRegister & 0b00010000 ) != 0;
    if ( is4KbChrMode ) {
      // Set upper 4 KB CHR bank
      u8 const bankIndex = shiftRegister & 0b00011111;
      chrBank4Hi = bankIndex;
    }
    // No 8 KiB mode for this target range
  } else if ( targetRegister == 3 ) // 0xE000-0xFFFF
  {
    // PRG banks
    u8 const prgBankMode = ( controlRegister >> 2 ) & 0b00000011;

    if ( prgBankMode == 0 || prgBankMode == 1 ) {
      // Set the 32 KiB prg bank

      // The masking logic first masks the shift register against 0xE,
      // which provides a range of even values from 0 to 14
      // It shifts this result right by 1, which is equivalent to dividing by 2
      // giving the final bank ranges from 0 to 7
      u8 const bankIndex = ( shiftRegister & 0b00001110 ) >> 1;
      prgBank32 = bankIndex;
    } else if ( prgBankMode == 2 ) {
      // 16 KiB mode

      // The lower bank is fixed to 0
      prgBank16Lo = 0;

      // The high bank is derived from the lower 4 bits of the shift register,
      // giving it a range from 0 to 15
      prgBank16Hi = shiftRegister & 0b00001111;
    } else if ( prgBankMode == 3 ) {
      // 16 KiB mode

      // The lower bank is derived from the lower 4 bits of the shift register, ranges from 0
      // to 15
      prgBank16Lo = shiftRegister & 0b00001111;

      // the high bank is fixed to the last bank
      prgBank16Hi = GetPrgBankCount() - 1;
    }
  }

  // Reset the shift register and write count
  shiftRegister = 0;
  writeCount = 0;
}

/*
################################
||                            ||
||          PPU Read          ||
||                            ||
################################
*/
[[nodiscard]] u32 Mapper1::MapPpuAddr( u16 address )
{
  // If no chr rom banks, the address is directly mapped
  if ( GetChrBankCount() == 0 ) {
    return address;
  }

  bool const is4KbChrMode = ( controlRegister & 0b10000 ) != 0;
  if ( is4KbChrMode ) {
    // 4 KiB, read the lower 4 KiB bank
    if ( address >= 0x0000 && address <= 0x0FFF ) {
      const u32 bankOffset = chrBank4Lo * 0x1000;
      return bankOffset + ( address & 0x0FFF );
    }
    // 4 KiB, read the upper 4 KiB bank
    if ( address >= 0x1000 && address <= 0x1FFF ) {
      const u32 bankOffset = chrBank4Hi * 0x1000;
      return bankOffset + ( address & 0x0FFF );
    }
  } else {
    // 8 KiB, read the 8 KiB bank
    const u32 bankOffset = chrBank8 * 0x2000;
    return bankOffset + ( address & 0x1FFF );
  }

  return 0xFF;
}

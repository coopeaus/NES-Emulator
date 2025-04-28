#include "mappers/mapper2.h"
#include "global-types.h"
#include "mappers/mapper-base.h"
#include <stdexcept>

MirrorMode Mapper2::GetMirrorMode()
{
  return _mirrorMode;
}

/*
################################
||                            ||
||          CPU Read          ||
||                            ||
################################
*/
[[nodiscard]] u32 Mapper2::MapPrgOffset( u16 address )
{
  /**
   * @details
   * - last 16 KiB bank is fixed at 0xC000-0xFFFF
   * - lower 16 KiB bank is swappable at 0x8000-0xBFFF
   * - select lower 16 KiB bank with _prg_bank_16_lo
   */

  if ( address >= 0x8000 && address <= 0xBFFF ) {
    u32 const bankOffset = _prgBank16Lo * 0x4000;
    return bankOffset + ( address & 0x3FFF );
  }

  if ( address >= 0xC000 && address <= 0xFFFF ) {
    // Swap out the fixed upper 16 KiB bank.
    u32 const bankOffset = ( GetPrgBankCount() - 1 ) * 0x4000;
    return bankOffset + ( address & 0x3FFF );
  }

  // If out of PRG range
  throw std::runtime_error( "Address out of range in MapPrgOffset" );
}

/*
################################
||                            ||
||          CPU Write         ||
||                            ||
################################
*/
void Mapper2::HandleCPUWrite( u16 address, u8 data )
{
  /** @brief Handle writes to signal bank swapping
   * @details
   * ROM is normally read-only, but mapper 1 (and other mappers)
   * use writes to change the bank selection in the cartridge.
   *
   * Ex:
   *   HandleCPUWrite( 0x8000, 0x03);
   *      - This will assign the lower 16 KiB bank to the 3rd bank
   *        (0xC000-0xFFFF).
   */

  if ( address >= 0x8000 && address <= 0xFFFF ) {
    // Set the lower 16 KiB bank

    // UNROM only for now..
    _prgBank16Lo = data & 0b00000111;
  }
}

/*
################################
||                            ||
||          PPU Read          ||
||                            ||
################################
*/
[[nodiscard]] u32 Mapper2::MapChrOffset( u16 address )
{
  /**
   * @brief Translate PPU address. Mapper 2 only supports direct mapping
   */
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    return address;
  }
  return 0xFF;
}

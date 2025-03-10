#include "mappers/mapper-base.h"
#include "mappers/mapper0.h"
#include <stdexcept>

[[nodiscard]] u32 Mapper0::TranslateCPUAddress( u16 address )
{
    /**
     * @brief Cartridges with Mapper 0 come in two fixed sizes: 16 KiB and 32 KiB
     * for 16 KiB cartridges, the second half is a mirror of the first half.
     */

    if ( address >= 0x8000 && address <= 0xFFFF ) {
        // 32 KiB PRG ROM
        if ( GetPrgBankCount() == 2 ) {
            // Map directly to PRG ROM, starting at 0x0000
            return address - 0x8000;
        }

        // 16 KiB PRG ROM
        if ( GetPrgBankCount() == 1 ) {
            // Map to 0, with mirroring after 16 KiB
            return ( address - 0x8000 ) % ( 16 * 1024 );
        }

        // Unsupported PRG ROM size
        throw std::runtime_error( "Mapper0:Unsupported PRG ROM size for Mapper 0" );
    }
    throw std::runtime_error( "Address out of range in TranslateCPUAddress" );
}

[[nodiscard]] auto Mapper0::TranslatePPUAddress( u16 address ) -> u32
{
    /**
     * @brief Translate PPU address. Mapper 0 doesn't redirect PPU address in any way
     */
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        return address;
    }
    return 0xFF;
}

void Mapper0::HandleCPUWrite( u16 address, u8 data )
{
    /**
     * @brief Handle CPU write. Mapper 0 doesn't do CPU writes
     */
    (void) address;
    (void) data;
}

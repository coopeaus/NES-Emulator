#include "mapper3.h"
#include "cartridge-header.h"
#include "mappers/mapper-base.h"
#include "global-types.h"

Mapper3::Mapper3( iNes2Instance iNesHeader ) : Mapper( iNesHeader )
{
}

u32 Mapper3::TranslateCPUAddress( u16 address )
{
    // CNROM: PRG is fixed, typically 32KB at $8000-$FFFF
    // Map $8000-$FFFF to PRG-ROM directly
    if ( address >= 0x8000 ) {
        // If only 16KB PRG, mirror it
        int const prgBankCount = GetPrgBankCount();
        u32       prgAddr = address - 0x8000;
        if ( prgBankCount == 1 ) // 16KB
            prgAddr = prgAddr % 0x4000;
        return prgAddr;
    }
    // Should not happen for CNROM
    return 0;
}

u32 Mapper3::TranslatePPUAddress( u16 address )
{
    // CNROM: CHR is banked in 8KB units
    if ( address < 0x2000 ) {
        u32 const chrBankCount = GetChrBankCount();
        u32 const bank = _chrBank % chrBankCount;
        return ( bank * 0x2000 ) + address;
    }
    // Not CHR address
    return address;
}

void Mapper3::HandleCPUWrite( u16 address, u8 data )
{
    // Any write to $8000-$FFFF sets CHR bank
    if ( address >= 0x8000 && address <= 0xFFFF ) {
        // Use a mask based on available CHR banks (support up to 8 banks)
        u8 const mask = ( GetChrBankCount() > 0 ) ? ( GetChrBankCount() - 1 ) : 0x03;
        _chrBank = data & mask;
    }
}

MirrorMode Mapper3::GetMirrorMode()
{
    // Use mirroring from header
    return static_cast<MirrorMode>( iNes.GetMirroring() );
}
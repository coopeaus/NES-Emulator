#include "bus.h"
#include "cartridge.h"
#include "global-types.h"
#include "ppu.h"
#include <iostream>

// Constructor to initialize the bus with a flat memory model
Bus::Bus() : cpu( this ), ppu( this ), cartridge( this )
{
}

/*
################################
||          CPU Read          ||
################################
*/
u8 Bus::Read( const u16 address, bool debugMode )
{
    if ( _useFlatMemory ) {
        return _flatMemory.at( address );
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        return _ram.at( address & 0x07FF );
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF ) {
        // ppu read will go here. For now, return from temp private member of bus
        const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        return ppu.CpuRead( ppuRegister, debugMode );
    }

    // APU
    if ( address == 0x4015 ) {
        return _apuIoMemory.at( address & 0x001F );
    }

    // Controller read
    if ( address >= 0x4016 && address <= 0x4017 ) {
        auto data = ( controllerState[address & 0x0001] & 0x80 ) > 0;
        controllerState[address & 0x0001] <<= 1;
        return data;
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF ) {
        return cartridge.Read( address );
    }

    // Unhandled address ranges return open bus value
    std::cout << "Unhandled read from address: " << std::hex << address << "\n";
    return 0xFF;
}

/*
################################
||          CPU Write         ||
################################
*/
void Bus::Write( const u16 address, const u8 data )
{

    if ( _useFlatMemory ) {
        _flatMemory.at( address ) = data;
        return;
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        _ram.at( address & 0x07FF ) = data;
        return;
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF ) {
        const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        ppu.CpuWrite( ppuRegister, data );
        return;
    }

    // PPU DMA: 0x4014
    if ( address == 0x4014 ) {
        dmaInProgress = true;
        dmaAddr = data << 8;
        dmaOffset = 0;
        return;
    }

    // APU
    if ( address >= 0x4000 && address <= 0x4015 ) {
        _apuIoMemory.at( address & 0x001F ) = data;
        return;
    }

    // Controller input
    if ( address >= 0x4016 && address <= 0x4017 ) {
        controllerState[address & 0x0001] = controller[address & 0x0001];
        return;
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF ) {
        cartridge.Write( address, data );
        return;
    }
    // Unhandled address ranges
    // Optionally log a warning or ignore
    std::cout << "Unhandled write to address: " << std::hex << address << "\n";
}

void Bus::ProcessDma()
{
    const u64 cycle = cpu.GetCycles();

    u8 const oamAddr = ppu.oamAddr;
    // Wait first read is on an odd cycle, wait it out.
    if ( dmaOffset == 0 && cycle % 2 == 1 ) {
        cpu.Tick();
        return;
    }

    // Read into OAM on even, load next byte on odd
    if ( cycle % 2 == 0 ) {
        auto data = Read( dmaAddr + dmaOffset );
        cpu.Tick();
        ppu.oam.data.at( ( oamAddr + dmaOffset ) & 0xFF ) = data;
        dmaOffset++;
    } else {
        dmaInProgress = dmaOffset < 256;
        cpu.Tick();
    }
}

void Bus::Clock()
{
    if ( dmaInProgress ) {
        ProcessDma();
    } else {
        cpu.DecodeExecute();
    }

    if ( ppu.nmiReady ) {
        ppu.nmiReady = false;
        cpu.NMI();
    }
}

/*
################################
||        Debug Methods       ||
################################
*/
[[nodiscard]] bool Bus::IsTestMode() const
{
    return _useFlatMemory;
}
void Bus::DebugReset()
{
    cpu.SetCycles( 0 );
    cpu.Reset();
    ppu.Reset();
}

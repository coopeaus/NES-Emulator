#include "bus.h"
#include "cartridge.h"
#include "ppu.h"
#include <iostream>
#include <memory>
#include <utility>

// Constructor to initialize the bus with a flat memory model
Bus::Bus() : cpu( this ), ppu( this )
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
        return _flatMemory[address];
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        return _ram[address & 0x07FF];
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF ) {
        // ppu read will go here. For now, return from temp private member of bus
        const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        return ppu.HandleCpuRead( ppuRegister, debugMode );
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F ) {
        // Handle reads from controller ports and other I/O
        // apu read will go here. For now, return from temp private member of bus
        return _apuIoMemory[address & 0x001F];
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF ) {
        return cartridge->Read( address );
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
        _flatMemory[address] = data;
        return;
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF ) {
        _ram[address & 0x07FF] = data;
        return;
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF ) {
        const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
        ppu.HandleCpuWrite( ppuRegister, data );
        return;
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F ) {
        _apuIoMemory[address & 0x001F] = data; // temp
        return;
    }

    // PPU DMA: 0x4014
    if ( address == 0x4014 ) {
        ppu.DmaTransfer( data );
        return;
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF ) {
        cartridge->Write( address, data );
        return;
    }
    // Unhandled address ranges
    // Optionally log a warning or ignore
    std::cout << "Unhandled write to address: " << std::hex << address << "\n";
}

/*
################################
||       Load Cartridge       ||
################################
*/
void Bus::LoadCartridge( std::shared_ptr<Cartridge> loadedCartridge )
{
    cartridge = std::move( loadedCartridge );
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

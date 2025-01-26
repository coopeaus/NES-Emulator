#include "bus.h"
#include "cartridge.h"
#include "ppu.h"
#include <iostream>
#include <memory>
#include <utility>

// Constructor to initialize the bus with a flat memory model
Bus::Bus( PPU *ppu, const bool use_flat_memory ) : _ppu( ppu ), _use_flat_memory( use_flat_memory )
{
    _ram.fill( 0 );
    _apu_io_memory.fill( 0 );
}

u8 Bus::Read( const u16 address ) const
{
    if ( _use_flat_memory )
    {
        return _flat_memory[address];
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        return _ram[address & 0x07FF];
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF )
    {
        // ppu read will go here. For now, return from temp private member of bus
        const u16 ppu_register = 0x2000 + ( address & 0x0007 );
        return _ppu->HandleCpuRead( ppu_register );
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F )
    {
        // Handle reads from controller ports and other I/O
        // apu read will go here. For now, return from temp private member of bus
        return _apu_io_memory[address & 0x001F];
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF )
    {
        return _cartridge->Read( address );
    }

    // Unhandled address ranges return open bus value
    std::cout << "Unhandled read from address: " << std::hex << address << "\n";
    return 0xFF;
}

void Bus::Write( const u16 address, const u8 data )
{
    if ( _use_flat_memory )
    {
        _flat_memory[address] = data;
        return;
    }

    // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
    if ( address >= 0x0000 && address <= 0x1FFF )
    {
        _ram[address & 0x07FF] = data;
        return;
    }

    // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
    if ( address >= 0x2000 && address <= 0x3FFF )
    {
        const u16 ppu_register = 0x2000 + ( address & 0x0007 );
        _ppu->HandleCpuWrite( ppu_register, data );
        return;
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F )
    {
        _apu_io_memory[address & 0x001F] = data; // temp
        return;
    }

    // 4020 and up is cartridge territory
    if ( address >= 0x4020 && address <= 0xFFFF )
    {
        _cartridge->Write( address, data );
        return;
    }
    // Unhandled address ranges
    // Optionally log a warning or ignore
    std::cout << "Unhandled write to address: " << std::hex << address << "\n";
}

void Bus::LoadCartridge( std::shared_ptr<Cartridge> cartridge ) { _cartridge = std::move( cartridge ); }

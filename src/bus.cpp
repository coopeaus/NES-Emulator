#include "bus.h"
#include <iostream>

// Constructor to initialize the bus with a flat memory model
Bus::Bus( const bool use_flat_memory ) : _use_flat_memory( use_flat_memory )
{
    _ram.fill( 0 );
    _ppu_memory.fill( 0 );
    _apu_io_memory.fill( 0 );
    _expansion_rom_memory.fill( 0 );
    _sram_memory.fill( 0 );
    _prg_rom_memory.fill( 0 );
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
        return _ppu_memory[ppu_register];
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F )
    {
        // Handle reads from controller ports and other I/O
        // apu read will go here. For now, return from temp private member of bus
        return _apu_io_memory[address & 0x001F];
    }

    // Expansion ROM: 0x4020 - 0x5FFF (if applicable)
    if ( address >= 0x4020 && address <= 0x5FFF )
    {
        // This will be read from the cartridge, it's rarely used
        // Normally, if there's no cartridge, the convention is to return 0xFF
        // For now, this memory will be read from a temp private member of bus
        return _expansion_rom_memory[address - 0x4020];
    }

    // SRAM (Save RAM): 0x6000 - 0x7FFF
    if ( address >= 0x6000 && address <= 0x7FFF )
    {
        // This will be read from the cartridge, if present
        // It represents battery-backed save data. Returns 0xFF if no cartridge
        // For now, this memory will be read from a temp private member of bus
        return _sram_memory[address - 0x6000];
    }

    // PRG ROM: 0x8000 - 0xFFFF
    if ( address >= 0x8000 && address <= 0xFFFF )
    {
        // This is PRG ROM, where the game code is stored
        // For now, this memory will be read from a temp private member of bus
        return _prg_rom_memory[address - 0x8000];
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
        _ppu_memory[ppu_register] = data; // temp
        return;
    }

    // APU and I/O Registers: 0x4000 - 0x401F
    if ( address >= 0x4000 && address <= 0x401F )
    {
        _apu_io_memory[address & 0x001F] = data; // temp
        return;
    }

    // Expansion ROM: 0x4020 - 0x5FFF (if applicable)
    if ( address >= 0x4020 && address <= 0x5FFF )
    {
        _expansion_rom_memory[address - 0x4020] = data; // temp
        return;
    }

    // SRAM (Save RAM): 0x6000 - 0x7FFF
    if ( address >= 0x6000 && address <= 0x7FFF )
    {
        _sram_memory[address - 0x6000] = data; // temp
        return;
    }

    // PRG ROM: 0x8000 - 0xFFFF (read only)
    if ( address >= 0x8000 && address <= 0xFFFF )
    {
        std::cout << "Attempted write to read-only PRG ROM address: " << std::hex << address
                  << "\n";
        return;
    }

    // Unhandled address ranges
    // Optionally log a warning or ignore
    std::cout << "Unhandled write to address: " << std::hex << address << "\n";
}


asdf

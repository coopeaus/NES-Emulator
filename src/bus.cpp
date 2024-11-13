#include "bus.h"
#include "cpu.h"

// Constructor to initialize the bus with all components (stubbed out in header)
Bus::Bus()
    : _ppu(nullptr), _apu(nullptr), _save_ram(nullptr), _cartridge(nullptr)
{
    // You can initialize the _flat_memory here if needed
    _flat_memory.fill(0);  // Just an example, if you want to initialize memory to 0
}

u8 Bus::Read(u16 address) const
{
    // Simplified read from flat memory.
    if (address <= 0x1FFF) {
        // System RAM: 0x0000 - 0x1FFF, mirrored every 2KB
        return _flat_memory[address % 0x0800];
    }
    if (address >= 0x2000 && address <= 0x3FFF && _ppu) {
        // PPU Registers: 0x2000 - 0x3FFF, mirrored every 8 bytes
        return _ppu->Read(address % 0x08);
    }
    if (address >= 0x4000 && address <= 0x401F && _apu) {
        // APU and I/O: 0x4000 - 0x401F
        return _apu->Read(address);
    }
    if (address >= 0x6000 && address <= 0x7FFF && _save_ram) {
        // SRAM (Save RAM): 0x6000 - 0x7FFF
        return _save_ram->Read(address - 0x6000);
    }
    if (address >= 0x8000 && address <= 0xFFFF && _cartridge) {
        // PRG ROM (Cartridge): 0x8000 - 0xFFFF
        return _cartridge->Read(address);
    }

    // Default case for unhandled addresses (if any)
    return _flat_memory[address];
}

void Bus::Write(u16 address, u8 data)
{
    // Simplified write to flat memory.
    if (address <= 0x1FFF) {
        // System RAM: 0x0000 - 0x1FFF, mirrored every 2KB
        _flat_memory[address % 0x0800] = data;
    }
    if (address >= 0x2000 && address <= 0x3FFF && _ppu) {
        // PPU Registers: 0x2000 - 0x3FFF, mirrored every 8 bytes
        _ppu->Write(address % 0x08, data);
    }
    if (address >= 0x4000 && address <= 0x401F && _apu) {
        // APU and I/O: 0x4000 - 0x401F
        _apu->Write(address, data);
    }
    if (address >= 0x6000 && address <= 0x7FFF && _save_ram) {
        // SRAM (Save RAM): 0x6000 - 0x7FFF
        _save_ram->Write(address - 0x6000, data);
    }
    if (address >= 0x8000 && address <= 0xFFFF && _cartridge) {
        // PRG ROM (Cartridge): Read-only, prevent writes
        _cartridge->Write(address, data);  // This will be ignored or handled in cartridge-specific code
    }

    // Default write to flat memory
    _flat_memory[address] = data;
}

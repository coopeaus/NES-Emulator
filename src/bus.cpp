#include "bus.h"
#include "cpu.h"
#include <array>
#include <cstdint>

// Temporary placeholder definitions for PPU, APU, SaveRAM, and Cartridge classes
class PPU
{
  public:
    uint8_t Read( uint16_t /*address*/ ) const { return 0; }   // Placeholder read method
    void    Write( uint16_t /*address*/, uint8_t /*data*/ ) {} // Placeholder write method
};

class APU {
public:
  [[nodiscard]] static uint8_t Read( uint16_t /*address*/ ) { return 0; }
  void                         Write( uint16_t /*address*/, uint8_t /*data*/ ) {}
};

class SaveRAM {
public:
    uint8_t Read(uint16_t /*address*/) const { return 0; }
    void Write(uint16_t /*address*/, uint8_t /*data*/) {}
};

class Cartridge {
public:
  [[nodiscard]] static uint8_t Read( uint16_t /*address*/ ) { return 0; }
  void                         Write( uint16_t /*address*/, uint8_t /*data*/ ) {}
};


// Constructor to initialize the bus with all components
Bus::Bus() = default;

uint8_t Bus::Read(uint16_t address) const
{
    if (address <= 0x1FFF) {
        // System RAM: 0x0000 - 0x1FFF, mirrored every 2KB
        return _flat_memory[address % 0x0800];
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        // PPU Registers: 0x2000 - 0x3FFF, mirrored every 8 bytes
        if (_ppu != nullptr) {
            return _ppu->Read(address % 0x08);
        }
    } else if (address >= 0x4000 && address <= 0x401F) {
        // APU and I/O: 0x4000 - 0x401F
        if (_apu != nullptr) {
            return _apu->Read(address);
        }
    } else if (address >= 0x6000 && address <= 0x7FFF) {
        // SRAM (Save RAM): 0x6000 - 0x7FFF
        if (_save_ram != nullptr) {
            return _save_ram->Read(address - 0x6000);
        }
    } else if (address >= 0x8000 && address <= 0xFFFF) {
        // PRG ROM (Cartridge): 0x8000 - 0xFFFF
        if (_cartridge != nullptr) {
            return _cartridge->Read(address);
        }
    }

    return _flat_memory[address]; // Default case for unhandled addresses
}

void Bus::Write(uint16_t address, uint8_t data)
{
    if (address <= 0x1FFF) {
        // System RAM: 0x0000 - 0x1FFF, mirrored every 2KB
        _flat_memory[address % 0x0800] = data;
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        // PPU Registers: 0x2000 - 0x3FFF, mirrored every 8 bytes
        if (_ppu != nullptr) {
            _ppu->Write(address % 0x08, data);
        }
    } else if (address >= 0x4000 && address <= 0x401F) {
        // APU and I/O: 0x4000 - 0x401F
        if (_apu != nullptr) {
            _apu->Write(address, data);
        }
    } else if (address >= 0x6000 && address <= 0x7FFF) {
        // SRAM (Save RAM): 0x6000 - 0x7FFF
        if (_save_ram != nullptr) {
            _save_ram->Write(address - 0x6000, data);
        }
    } else if (address >= 0x8000 && address <= 0xFFFF) {
        // PRG ROM (Cartridge): Read-only, prevent writes
        if (_cartridge != nullptr) {
            _cartridge->Write(address, data);  // Placeholder write handling
        }
    } else {
        _flat_memory[address] = data; // Default write to flat memory
    }
}

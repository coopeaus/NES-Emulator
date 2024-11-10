// bus.cpp
#include "bus.h"
#include "cpu.h"

Bus::Bus() = default;

u8 Bus::Read( u16 address ) const
{
    // Simplfied read.
    return _flat_memory[address];

    // TODO: System RAM reads: 0x0000 - 0x1FFF, mirrored every 2KB

    // TODO: PPU reads: 0x2000 - 0x3FFF, mirrored every 8 bytes

    // TODO: APU and I/O reads: 0x4000 - 0x401F

    // TODO: Expansion ROM: 0x4020 - 0x5FFF

    // TODO: SRAM (Save RAM): 0x6000 - 0x7FFF

    // TODO: PRG ROM (Cartridge): 0x8000 - 0xFFFF
}

void Bus::Write( u16 address, u8 data )
{
    // Simplified write.
    _flat_memory[address] = data;

    // TODO: System RAM writes: 0x0000 - 0x1FFF, mirrored every 2KB

    // TODO: PPU writes: 0x2000 - 0x3FFF, mirrored every 8 bytes

    // TODO: APU and I/O writes: 0x4000 - 0x401F

    // TODO: Expansion ROM writes: 0x4020 - 0x5FFF

    // TODO: SRAM (Save RAM) writes: 0x6000 - 0x7FFF

    // TODO: PRG ROM (Cartridge) writes: Read-only, prevent writes
}

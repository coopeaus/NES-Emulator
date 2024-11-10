// cpu.cpp

#include "bus.h"
CPU::CPU( Bus *bus ) : _bus( bus ) {} // initialize CPU with pointer to bus

// Pass off reads and writes to the bus
auto CPU::Read( u16 address ) const -> u8 { return _bus->Read( address ); }
void CPU::Write( u16 address, u8 data ) { _bus->Write( address, data ); }

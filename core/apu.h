#pragma once
#include "utils.h"
#include "global-types.h"

class Bus;
class APU
{
public:
  APU( Bus *bus );
  Bus *bus;

  template <class Archive> void serialize( Archive & /*ar*/ ) {} // NOLINT

  /*
  ################################
  ||      CPU Read / Write      ||
  ################################
  */
  u8 HandleCpuRead( u16 addr )
  {
    using utils::between;
    if ( between( addr, 0x4000, 0x4003 ) ) {
      Pulse1Read();
    } else if ( between( addr, 0x4004, 0x4007 ) ) {
      Pulse2Read();
    } else if ( between( addr, 0x4008, 0x400B ) ) {
      TriangleRead();
    } else if ( between( addr, 0x400C, 0x400F ) ) {
      NoiseRead();
    } else if ( between( addr, 0x4010, 0x4013 ) ) {
      DMCRead();
    } else if ( addr == 0x4015 ) {
      StatusRead();
    } else if ( addr == 0x4017 ) {
      FrameCounterRead();
    }

    // placeholder
    return 0xFF;
  }

  void HandleCpuWrite( u16 address, u8 data )
  {
    using utils::between;
    if ( between( address, 0x4000, 0x4003 ) ) {
      Pulse1Write( data );
    } else if ( between( address, 0x4004, 0x4007 ) ) {
      Pulse2Write( data );
    } else if ( between( address, 0x4008, 0x400B ) ) {
      TriangleWrite( data );
    } else if ( between( address, 0x400C, 0x400F ) ) {
      NoiseWrite( data );
    } else if ( between( address, 0x4010, 0x4013 ) ) {
      DMCWrite( data );
    } else if ( address == 0x4015 ) {
    } else if ( address == 0x4017 ) {
      FrameCounterWrite( data );
    }
  }

  /*
  ################################
  ||         APU Methods        ||
  ################################
  */

  // Placeholders
  void Pulse1Read() {}
  void Pulse2Read() {}
  void TriangleRead() {}
  void NoiseRead() {}
  void StatusRead() {}
  void DMCRead() {}
  void FrameCounterRead() {}

  // NOLINTBEGIN(readability-convert-member-functions-to-static)
  void Pulse1Write( u8 data ) { (void) data; };
  void Pulse2Write( u8 data ) { (void) data; };
  void TriangleWrite( u8 data ) { (void) data; };
  void NoiseWrite( u8 data ) { (void) data; };
  void DMCWrite( u8 data ) { (void) data; };
  void FrameCounterWrite( u8 data ) { (void) data; };
  // NOLINTEND(readability-convert-member-functions-to-static)
};

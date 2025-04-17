#pragma once
#include "cpu.h"
#include <cstdint>
#include <array>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;

using namespace std;

class APU
{
  public:
    explicit APU( Bus *bus );

    /*
    ################################
    ||           Getters          ||
    ################################
    */

    u8 GetApuStatus() const { return apuStatus.value; }
    u8 GetOamAddr() const { return oamAddr; }
    u8 GetOamData() const { return oamData; }
    u8 GetApuScroll() const { return apuScroll; }
    u8 GetApuAddr() const { return apuAddr; }
    u8 GetApuData() const { return apuData; }

    bool GetAddrLatch() const { return addrLatch; }

    /*
    ################################
    ||           Setters          ||
    ################################
    */
    void SetScanline( s16 val ) { scanline = val; }

    /*
    ################################
    ||      CPU Read / Write      ||
    ################################
    */
    [[nodiscard]] u8 HandleCpuRead( u16 address, bool debugMode = false );
    void             HandleCpuWrite( u16 address, u8 data );

    /*
    ################################
    ||       Internal Reads       ||
    ################################
    */
    [[nodiscard]] u8 Read( u16 addr );

    /*
    ################################
    ||       Internal Writes      ||
    ################################
    */
    void Write( u16 addr, u8 data );

    /*
    ################################
    ||         APU Methods        ||
    ################################
    */
    void Reset()
    {
        cycle = 4;
        apuStatus.value = 0x00;
        oamAddr = 0x00;
        oamData = 0x00;
        apuAddr = 0x00;
        apuData = 0x00;
    }

    /*
    ################################
    ||        Debug Methods       ||
    ################################
    */
    void EnableJsonTestMode() { isDisabled = true; }
    void DisableJsonTestMode() { isDisabled = false; }

    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
    bool failedPaletteRead = false;
    int  systemPaletteIdx = 0;
    int  maxSystemPalettes = 3;

    /*
    ################################
    ||      Global Variables      ||
    ################################
    */
    s16            scanline = 0;
    u16            cycle = 4;
    u64            frame = 1;
    bool           isRenderingEnabled = false;
    bool           preventVBlank = false;
    array<u32, 64> nesPaletteRgbValues{};

    /*
    ################################
    ||       Debug Variables      ||
    ################################
    */
    bool isDisabled = false;

    /*
    ################################
    ||         Peripherals        ||
    ################################
    */
    Bus *bus;

    /*
    ################################
    ||    CPU-facing Registers    ||
    ################################
    */

    union APUSTATUS {
        struct {
            u8 enableDMC : 4;
            u8 noise : 5;
            u8 triangle : 6;
            u8 pulse2 : 7;
            u8 pulse1 : 8;
        } bit;
        u8 value = 0x00;
    };

    APUSTATUS apuStatus;        // $2002
    u8        oamAddr = 0x00;   // $2003
    u8        oamData = 0x00;   // $2004
    u8        apuScroll = 0x00; // $2005
    u8        apuAddr = 0x00;   // $2006
    u8        apuData = 0x00;   // $2007
    // $4014: OAM DMA, handled in bus read/write, see bus.cpp

    /*
    ################################################################
    ||                     Internal Registers                     ||
    ################################################################
    */

    // Used by _apuScroll and _apuAddr for two-write operations
    bool addrLatch = false;

    // Stores last data written to _apuData
    u8 apuDataBuffer = 0x00;

    /*
    ################################################################
    ||                  Internal Memory Locations                 ||
    ################################################################
    */
    array<u8, 256> oam{};
};

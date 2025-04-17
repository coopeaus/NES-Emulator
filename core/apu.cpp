#include "apu.h"
#include "bus.h"
#include "cartridge.h" // NOLINT

APU::APU( Bus *bus ) : bus( bus ) // NOLINT
{
}

/*
################################
||                            ||
||       Handle CPU Read      ||
||                            ||
################################
*/
[[nodiscard]] u8 APU::HandleCpuRead( u16 address, bool debugMode ) // NOLINT
{

    /* @brief: CPU reads to the APU
     * @details:
     * @source: https://www.nesdev.org/wiki/APU_registers
     */

    // Write-only registers:
    // PULSE 1: 4000 (DDLC NNNN	Duty, loop envelope/disable length counter, constant volume, envelope
    // period/volume), PULSE 1: 4001 (EPPP NSSS	Sweep unit: enabled, period, negative, shift count) PULSE 1:
    // 4002 (LLLL LLLL	Timer low) PULSE 1: 4003 (LLLL LHHH	Length counter load, timer high (also resets
    // duty and starts envelope)

    // PULSE 2: 4004 (DDLC NNNN	Duty, loop envelope/disable length counter, constant volume, envelope
    // period/volume), PULSE 2: 4005 (EPPP NSSS	Sweep unit: enabled, period, negative, shift count) PULSE 2:
    // 4006 (LLLL LLLL	Timer low) PULSE 2: 4007 (LLLL LHHH	Length counter load, timer high (also resets
    // duty and starts envelope)

    // TRIANGLE: 4008 (CRRR RRRR Length counter disable/linear counter control, linear counter reload value),
    // TRIANGLE: 400A (LLLL LLLL Timer low)
    // TRIANGLE: 400B (LLLL LHHH Length counter load, timer high (also reloads linear counter)

    // NOISE: 400C (--LC NNNN	Loop envelope/disable length counter, constant volume, envelope
    // period/volume), NOISE: 400E (L--- PPPP	Loop noise, noise period) NOISE: 400F (LLLL L---	Length
    // counter load (also starts envelope)

    // DMC: 4010 (IL-- FFFF	IRQ enable, loop sample, frequency index),
    // DMC: 4011 (-DDD DDDD	Direct load)
    // DMC: 4012 (AAAA AAAA	Sample address %11AAAAAA.AA000000)
    // DMC: 4013 (LLLL LLLL	Sample length %0000LLLL.LLLL0001)

    // Read-Write Registers
    // 4015	(---D NT21	Control: DMC enable, length counter enables: noise, triangle, pulse 2, pulse 1
    // (write)) 4016	(IF-D NT21	Status: DMC interrupt, frame interrupt, length counter status: noise,
    // triangle, pulse 2, pulse 1 (read)) 4017	(SD-- ----	Frame counter: 5-frame sequence, disable frame
    // interrupt (write))

    // 4000: APU Status Register
    if ( address == 0x4015 ) {
        // STATUS CHANNEL ---D NT21	Enable DMC (D), noise (N), triangle (T), and pulse channels (2/1)
        return address;
    }
    return 0xFF;
}

/*
################################
||                            ||
||      Handle CPU Write      ||
||                            ||
################################
*/

void APU::HandleCpuWrite( u16 address, u8 data ) // NOLINT
{
    /* @brief: CPU writes to the APU
     */

    (void) data; // NOTE: Remove after using data

    switch ( address ) {
        // 4000: APU PULSE
        case 0x4000: // NOLINT
        {
            break;
        }
        // 4001: APU PULSE
        case 0x4001: {

            break;
        }
        // 4002: APU PULSE
        case 0x4002:
            break;
        // 4003: APU PULSE
        case 0x4003: // NOLINT
        {
            break;
        }
        // 4004: APU PULSE
        case 0x4004: {

            break;
        }
        // 4005: APU PULSE
        case 0x4005: // NOLINT
        {
            break;
        }
        // 4006: APU PULSE
        case 0x4006: {
            break;
        }
        // 4007: APU PULSE
        case 0x4007: {
            break;
        }
        case 0x4008: {
            break;
        }
        case 0x400A: {
            break;
        }
        case 0x400B: {
            break;
        }
        case 0x400C: {
            break;
        }
        case 0x400E: {
            break;
        }
        case 0x400F: {
            break;
        }
        case 0x4010: {
            break;
        }
        case 0x4011: {
            break;
        }
        case 0x4012: {
            break;
        }
        case 0x4013: {
            break;
        }
        case 0x4015: {
            break;
        }
        case 0x4016: {
            break;
        }
        case 0x4017: {
            break;
        }
        default:
            break;
    }
}

/*
################################
||                            ||
||          APU Read          ||
||                            ||
################################
*/

[[nodiscard]] u8 APU::Read( u16 address ) // NOLINT
{
    /*@brief: Internal APU reads to the cartridge
     */
    // $0000-$1FFF: Pattern Tables
    if ( address ) {
        /* Pattern table data is read from the cartridge */
        return bus->cartridge.Read( address );
    }

    return 0xFF;
}

/*
################################
||                            ||
||          APU Write         ||
||                            ||
################################
*/
void APU::Write( u16 address, u8 data ) // NOLINT
{
    /*@brief: Internal APU reads to the cartridge
     */

    (void) data;
    address &= 0x4000;

    if ( address >= 0x0000 && address <= 0x4018 ) {
        /* Pattern table data is written to the cartridge */
        bus->cartridge.Write( address, data );
        return;
    }
}

/*
################################
||                            ||
||       APU Cycle Tick       ||
||                            ||
################################
*/

/*
################################
||                            ||
||           Helpers          ||
||                            ||
################################
*/

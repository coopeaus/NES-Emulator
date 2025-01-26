#include <gtest/gtest.h>
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "utils.h"
#include "cartridge.h"
#include <fstream>
#include <regex>
#include <vector>
#include <iostream>
#include <memory>
using namespace std;

/*
################################################
||                                            ||
||                 Nestest.nes                ||
||                                            ||
################################################
*/

TEST( RomTests, Nestest )
{
    PPU ppu;
    Bus bus(&ppu);
    CPU cpu( &bus );

    // Create a shared pointer to Cartridge
    shared_ptr<Cartridge> cartridge = make_shared<Cartridge>( "tests/roms/nestest.nes" );

    // Pass the shared pointer to LoadCartridge
    bus.LoadCartridge( cartridge );

    cpu.Reset();

    // Open the log file for writing output data
    ofstream actual_output( "tests/output/my_nestest-log.txt" );

    // color codes
    const std::string red = "\033[31m";
    const std::string reset = "\033[0m";

    if ( !actual_output.is_open() )
    {
        cerr << "Failed to open output/my_nestest-log.txt for writing\n";
        FAIL() << "Cannot open log file.";
        return;
    }

    // Pattern for nestest log output
    regex pattern(
        R"(^([A-F0-9]{4}).*?A:([A-F0-9]{2}).*?X:([A-F0-9]{2}).*?Y:([A-F0-9]{2}).*?P:([A-F0-9]{2}).*?SP:([A-F0-9]{2}).*?,\s*(\d+).*?CYC:(\d+))",
        regex_constants::ECMAScript );

    utils::MatchResults matches = utils::parseLog( "tests/logs/nestest-log.txt", pattern, 8 );

    struct NestestLogInfo
    {
        u16 pc;
        u8  a;
        u8  x;
        u8  y;
        u8  p;
        u8  sp;
        // s16 ppu_cycles;
        u64 cpu_cycles;
    };
    std::vector<NestestLogInfo> expected_lines;
    for ( const auto &match : matches )
    {
        NestestLogInfo entry{};
        entry.pc = static_cast<u16>( stoul( match[0], nullptr, 16 ) );
        entry.a = static_cast<u8>( stoul( match[1], nullptr, 16 ) );
        entry.x = static_cast<u8>( stoul( match[2], nullptr, 16 ) );
        entry.y = static_cast<u8>( stoul( match[3], nullptr, 16 ) );
        entry.p = static_cast<u8>( stoul( match[4], nullptr, 16 ) );
        entry.sp = static_cast<u8>( stoul( match[5], nullptr, 16 ) );
        // entry.ppu_cycles = static_cast<s16>( stoul( match[6], nullptr, 10 ) );
        entry.cpu_cycles = stoull( match[7], nullptr, 10 );
        expected_lines.push_back( entry );
    }

    // Initialize CPU state with the first expected entry
    cpu.SetProgramCounter( expected_lines[0].pc );
    cpu.SetAccumulator( expected_lines[0].a );
    cpu.SetXRegister( expected_lines[0].x );
    cpu.SetYRegister( expected_lines[0].y );
    cpu.SetStatusRegister( expected_lines[0].p );
    cpu.SetStackPointer( expected_lines[0].sp );
    cpu.SetCycles( expected_lines[0].cpu_cycles );

    size_t line_index = 0;
    bool   did_fail = false;

    // Execute the CPU until the end of the test ROM

    while ( line_index < expected_lines.size() )
    {
        // Save output to our own log file
        actual_output << cpu.LogLineAtPC() << '\n';

        // Get the expected line
        const auto &expected = expected_lines[line_index];

        // Compare all the relevant values with expected

        if ( cpu.GetProgramCounter() != expected.pc )
        {
            did_fail = true;
            cerr << '\n';
            cerr << red << ( line_index + 1 ) << ": PC mismatch\n" << reset;
            cerr << "Expected: " << utils::toHex( expected.pc, 4 ) << "    "
                 << "Actual: " << utils::toHex( cpu.GetProgramCounter(), 4 ) << '\n';
            cerr << '\n';
        }

        if ( cpu.GetAccumulator() != expected.a )
        {
            did_fail = true;
            cerr << '\n';
            std::cerr << red << ( line_index + 1 ) << ": A mismatch\n" << reset;
            std::cerr << "Expected: " << utils::toHex( expected.a, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetAccumulator(), 2 ) << '\n';
            cerr << '\n';
        }

        if ( cpu.GetXRegister() != expected.x )
        {
            did_fail = true;
            cerr << '\n';
            std::cerr << red << ( line_index + 1 ) << ": X mismatch\n" << reset;
            std::cerr << "Expected: " << utils::toHex( expected.x, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetXRegister(), 2 ) << '\n';
            cerr << '\n';
        }

        if ( cpu.GetYRegister() != expected.y )
        {
            did_fail = true;
            cerr << '\n';
            std::cerr << red << ( line_index + 1 ) << ": Y mismatch\n" << reset;
            std::cerr << "Expected: " << utils::toHex( expected.y, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetYRegister(), 2 ) << '\n';
            cerr << '\n';
        }

        if ( cpu.GetStatusRegister() != expected.p )
        {
            did_fail = true;
            cerr << '\n';
            std::cerr << red << ( line_index + 1 ) << ": P mismatch\n" << reset;
            std::cerr << "Expected: " << utils::toHex( expected.p, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetStatusRegister(), 2 ) << '\n';
            cerr << '\n';
        }

        if ( cpu.GetStackPointer() != expected.sp )
        {
            did_fail = true;
            cerr << '\n';
            std::cerr << red << ( line_index + 1 ) << ": SP mismatch\n" << reset;
            std::cerr << "Expected: " << utils::toHex( expected.sp, 2 ) << "    "
                      << "Actual: " << utils::toHex( cpu.GetStackPointer(), 2 ) << '\n';
            cerr << '\n';
        }

        if ( cpu.GetCycles() != expected.cpu_cycles )
        {
            did_fail = true;
            cerr << '\n';
            std::cerr << red << ( line_index + 1 ) << ": Cycles mismatch\n" << reset;
            std::cerr << "Expected: " << expected.cpu_cycles << "    "
                      << "Actual: " << cpu.GetCycles() << '\n';
            cerr << '\n';
        }

        if ( did_fail )
        {

            FAIL() << red << "Mismatch detected at line " << ( line_index + 1 ) << reset;
            break;
        }

        cpu.Tick(); // Run one CPU cycle
        line_index++;
    }

    if ( did_fail )
    {
        std::cerr << red << "Nestest failed.\n" << reset;
    }
    else
    {

        std::string green = "\033[32m";
        std::cout << green << "Nestest passed.\n" << reset;
    }

    actual_output.close();
}

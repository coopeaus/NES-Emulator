#include <gtest/gtest.h>
#include "bus.h"
#include "paths.h"
#include "cpu.h"
#include "ppu.h"
#include "utils.h"
#include "cartridge.h"
#include <fstream>
#include <regex>
#include <vector>
#include <iostream>
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
    Bus  bus;
    CPU &cpu = bus.cpu;
    PPU &ppu = bus.ppu;

    std::string romFile = std::string( paths::roms() ) + "/nestest.nes";
    bus.cartridge.LoadRom( romFile );

    cpu.Reset();

    // Open the log file for writing output data
    ofstream actualOutput( "tests/output/my_nestest-log.txt" );

    // color codes
    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string reset = "\033[0m";

    if ( !actualOutput.is_open() ) {
        cerr << "Failed to open output/my_nestest-log.txt for writing\n";
        FAIL() << "Cannot open log file.";
        return;
    }

    // Pattern for nestest log output
    regex pattern(
        R"(^([A-F0-9]{4}).*?A:([A-F0-9]{2}).*?X:([A-F0-9]{2}).*?Y:([A-F0-9]{2}).*?P:([A-F0-9]{2}).*?SP:([A-F0-9]{2}).*?,\s*(\d+).*?CYC:(\d+))",
        regex_constants::ECMAScript );

    utils::MatchResults matches = utils::parseLog( "tests/logs/nestest-log.txt", pattern, 8 );

    struct NestestLogInfo {
        u16 pc;
        u8  a;
        u8  x;
        u8  y;
        u8  p;
        u8  sp;
        s16 ppuCycles;
        u64 cpuCycles;
    };
    std::vector<NestestLogInfo> expectedLines;
    for ( const auto &match : matches ) {
        NestestLogInfo entry{};
        entry.pc = static_cast<u16>( stoul( match[0], nullptr, 16 ) );
        entry.a = static_cast<u8>( stoul( match[1], nullptr, 16 ) );
        entry.x = static_cast<u8>( stoul( match[2], nullptr, 16 ) );
        entry.y = static_cast<u8>( stoul( match[3], nullptr, 16 ) );
        entry.p = static_cast<u8>( stoul( match[4], nullptr, 16 ) );
        entry.sp = static_cast<u8>( stoul( match[5], nullptr, 16 ) );
        entry.ppuCycles = static_cast<s16>( stoul( match[6], nullptr, 10 ) );
        entry.cpuCycles = stoull( match[7], nullptr, 10 );
        expectedLines.push_back( entry );
    }

    // Initialize CPU state with the first expected entry
    cpu.SetProgramCounter( expectedLines[0].pc );
    cpu.SetAccumulator( expectedLines[0].a );
    cpu.SetXRegister( expectedLines[0].x );
    cpu.SetYRegister( expectedLines[0].y );
    cpu.SetStatusRegister( expectedLines[0].p );
    cpu.SetStackPointer( expectedLines[0].sp );
    cpu.SetCycles( expectedLines[0].cpuCycles );

    size_t lineIndex = 0;
    bool   didFail = false;

    // Execute the CPU until the end of the test ROM
    std::cout << "PC: " << utils::toHex( cpu.GetProgramCounter(), 4 ) << '\n';

    while ( lineIndex < expectedLines.size() ) {
        try {
            // Save output to our own log file
            actualOutput << cpu.LogLineAtPC() << '\n';

            // Get the expected line
            const auto &expected = expectedLines[lineIndex];

            // Compare all the relevant values with expected

            if ( cpu.GetProgramCounter() != expected.pc ) {
                didFail = true;
                cerr << '\n';
                cerr << red << ( lineIndex + 1 ) << ": PC mismatch" << reset << '\n';
                cerr << "Expected: " << utils::toHex( expected.pc, 4 ) << "    "
                     << "Actual: " << utils::toHex( cpu.GetProgramCounter(), 4 ) << '\n';
                cerr << '\n';
            }

            if ( cpu.GetAccumulator() != expected.a ) {
                didFail = true;
                cerr << '\n';
                std::cerr << red << ( lineIndex + 1 ) << ": A mismatch" << reset << '\n';
                std::cerr << "Expected: " << utils::toHex( expected.a, 2 ) << "    "
                          << "Actual: " << utils::toHex( cpu.GetAccumulator(), 2 ) << '\n';
                cerr << '\n';
            }

            if ( cpu.GetXRegister() != expected.x ) {
                didFail = true;
                cerr << '\n';
                std::cerr << red << ( lineIndex + 1 ) << ": X mismatch" << reset << '\n';
                std::cerr << "Expected: " << utils::toHex( expected.x, 2 ) << "    "
                          << "Actual: " << utils::toHex( cpu.GetXRegister(), 2 ) << '\n';
                cerr << '\n';
            }

            if ( cpu.GetYRegister() != expected.y ) {
                didFail = true;
                cerr << '\n';
                std::cerr << red << ( lineIndex + 1 ) << ": Y mismatch" << reset << '\n';
                std::cerr << "Expected: " << utils::toHex( expected.y, 2 ) << "    "
                          << "Actual: " << utils::toHex( cpu.GetYRegister(), 2 ) << '\n';
                cerr << '\n';
            }

            if ( cpu.GetStatusRegister() != expected.p ) {
                didFail = true;
                cerr << '\n';
                std::cerr << red << ( lineIndex + 1 ) << ": P mismatch" << reset << '\n';
                std::cerr << "Expected: " << utils::toHex( expected.p, 2 ) << "    "
                          << "Actual: " << utils::toHex( cpu.GetStatusRegister(), 2 ) << '\n';
                cerr << '\n';
            }

            if ( cpu.GetStackPointer() != expected.sp ) {
                didFail = true;
                cerr << '\n';
                std::cerr << red << ( lineIndex + 1 ) << ": SP mismatch" << reset << '\n';
                std::cerr << "Expected: " << utils::toHex( expected.sp, 2 ) << "    "
                          << "Actual: " << utils::toHex( cpu.GetStackPointer(), 2 ) << '\n';
                cerr << '\n';
            }

            if ( cpu.GetCycles() != expected.cpuCycles ) {
                didFail = true;
                cerr << '\n';
                std::cerr << red << ( lineIndex + 1 ) << ": Cycles mismatch" << reset << '\n';
                std::cerr << "Expected: " << expected.cpuCycles << "    "
                          << "Actual: " << cpu.GetCycles() << '\n';
                cerr << '\n';
            }

            if ( didFail ) {
                string         actualLine = cpu.LogLineAtPC();
                NestestLogInfo expected = expectedLines[lineIndex];

                // Set the pc state to expected and then log the line, to match formatting
                cpu.SetProgramCounter( expected.pc );
                cpu.SetAccumulator( expected.a );
                cpu.SetXRegister( expected.x );
                cpu.SetYRegister( expected.y );
                cpu.SetStackPointer( expected.sp );
                cpu.SetStatusRegister( expected.p );
                cpu.SetCycles( expected.cpuCycles );
                ppu.SetScanline( expected.ppuCycles );
                ppu.SetCycles( expected.ppuCycles );

                string expectedLine = cpu.LogLineAtPC();

                std::cerr << red << "Mismatch detected at line " << ( lineIndex + 1 ) << reset << '\n';
                std::cerr << "\n";
                std::cerr << "Actual Vs. Expected:\n";
                std::cerr << red << actualLine << reset << '\n';
                std::cerr << green << expectedLine << reset << '\n';
                std::cerr << "\n";
                FAIL();
                break;
            }

            bus.Clock(); // Run one CPU cycle
            lineIndex++;
        } catch ( const std::exception &e ) {
            std::cerr << red << "Failed at line " << ( lineIndex + 1 ) << reset << '\n';
            std::cerr << "Exception: " << e.what() << '\n';
            FAIL();
        }
    }

    if ( didFail ) {
        std::cerr << red << "Nestest failed." << reset << '\n';
    } else {

        std::cout << green << "Nestest passed." << reset << '\n';
    }

    actualOutput.close();
}

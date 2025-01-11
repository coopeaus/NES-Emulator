#include <gtest/gtest.h>
#include "bus.h"
#include "cpu.h"
#include "cartridge.h"
#include <fstream>
#include <regex>
#include <vector>
#include <iostream>
#include <memory>
using namespace std;

auto toHex( uint32_t num, uint8_t width ) -> string
{
    string     hex_str( width, '0' );
    const char hex_chars[] = "0123456789ABCDEF";
    for ( int i = width - 1; i >= 0; --i, num >>= 4 )
    {
        hex_str[i] = hex_chars[num & 0xF];
    }
    return hex_str;
}

// Structure to hold expected CPU state from the log
struct ExpectedLine
{
    uint16_t        address;
    vector<uint8_t> opcode; // Changed to vector to handle multiple opcode bytes
    uint8_t         a;
    uint8_t         x;
    uint8_t         y;
    uint8_t         p;
    uint8_t         sp;
    uint64_t        cycles;
};

/*
################################################
||                                            ||
||                 Nestest.nes                ||
||                                            ||
################################################
*/
TEST( RomTests, Nestest )
{
    Bus bus;
    CPU cpu( &bus );

    // Create a shared pointer to Cartridge
    shared_ptr<Cartridge> cartridge = make_shared<Cartridge>( "tests/roms/nestest.nes" );

    // Pass the shared pointer to LoadCartridge
    bus.LoadCartridge( cartridge );

    cpu.Reset();

    // Open the log file for writing disassembly data
    ofstream log( "tests/output/my_nestest-log.txt" );
    if ( !log.is_open() )
    {
        cerr << "Failed to open output/my_nestest-log.txt for writing\n";
        FAIL() << "Cannot open log file.";
        return;
    }

    // Open the nestest.log file for reading expected CPU states
    ifstream nestest_log( "tests/logs/nestest-log.txt" );
    if ( !nestest_log.is_open() )
    {
        cerr << "Failed to open nestest-log.txt for reading\n";
        FAIL() << "Cannot open nestest log file.";
        return;
    }

    // Define a bit of regex magic to grab groups of hex bytes
    regex line_pattern(
        R"(^([A-F0-9]{4})\s+([A-F0-9]{2})\s.{39}A:([A-F0-9]{2})\s*X:([A-F0-9]{2})\s*Y:([A-F0-9]{2})\s*P:([A-F0-9]{2})\s*SP:([A-F0-9]{2})\s*.{12}CYC:(\d+))",
        regex_constants::ECMAScript );

    // Read in the expected lines into a vector, which we will use to compare against our own
    // execution
    vector<ExpectedLine> expected_lines;

    string line;
    size_t line_number = 0;
    while ( getline( nestest_log, line ) )
    {
        ++line_number;
        smatch match;
        // here comes the regex magic
        if ( regex_match( line, match, line_pattern ) )
        {
            try
            {
                if ( match.size() != 9 )
                {
                    throw runtime_error( "Regex groups missing data" );
                }

                // Address
                ExpectedLine entry{};
                entry.address = static_cast<uint16_t>( stoul( match[1].str(), nullptr, 16 ) );

                // Parse opcode bytes
                vector<uint8_t> opcode_bytes;
                istringstream   opcode_stream( match[2].str() );
                string          byte_str;
                while ( opcode_stream >> byte_str )
                {
                    uint8_t byte = static_cast<uint8_t>( stoul( byte_str, nullptr, 16 ) );
                    opcode_bytes.push_back( byte );
                }
                entry.opcode = opcode_bytes;

                // Registers and the rest of the data
                entry.a = static_cast<uint8_t>( stoul( match[3].str(), nullptr, 16 ) );
                entry.x = static_cast<uint8_t>( stoul( match[4].str(), nullptr, 16 ) );
                entry.y = static_cast<uint8_t>( stoul( match[5].str(), nullptr, 16 ) );
                entry.p = static_cast<uint8_t>( stoul( match[6].str(), nullptr, 16 ) );
                entry.sp = static_cast<uint8_t>( stoul( match[7].str(), nullptr, 16 ) );
                entry.cycles = static_cast<uint64_t>( stoul( match[8].str(), nullptr, 10 ) );

                expected_lines.push_back( entry );
            }
            catch ( const exception &e )
            {
                cerr << "Failed to parse line " << line_number << ": " << line
                     << "\nException: " << e.what() << '\n';
                FAIL() << "Parsing failed at line " << line_number;
                return;
            }
        }
        else
        {
            cerr << "Regex did not match line " << line_number << ": " << line << '\n';
        }
    }

    if ( expected_lines.empty() )
    {
        std::cerr << "No expected lines were parsed. Check your regex and input file.\n";
        FAIL() << "No parsed lines.";
        return;
    }

    // Initialize CPU state with the first expected entry
    cpu.SetProgramCounter( expected_lines[0].address );
    cpu.SetAccumulator( expected_lines[0].a );
    cpu.SetXRegister( expected_lines[0].x );
    cpu.SetYRegister( expected_lines[0].y );
    cpu.SetStatusRegister( expected_lines[0].p );
    cpu.SetStackPointer( expected_lines[0].sp );
    cpu.SetCycles( expected_lines[0].cycles );

    size_t line_index = 0;
    bool   did_fail = false;

    // Execute the CPU until the end of the test ROM

    while ( line_index < expected_lines.size() )
    {
        // Save output to our own log file
        log << cpu.DisassembleAtPC() << '\n';

        // Get the expected line
        const auto &expected = expected_lines[line_index];

        // Compare all the relevant values with expected

        if ( cpu.GetProgramCounter() != expected.address )
        {
            did_fail = true;
            cerr << ( line_index + 1 ) << ": PC mismatch\n";
            cerr << "Expected: " << toHex( expected.address, 4 ) << "    "
                 << "Actual: " << toHex( cpu.GetProgramCounter(), 4 ) << '\n';
        }

        if ( cpu.GetAccumulator() != expected.a )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": A mismatch\n";
            std::cerr << "Expected: " << toHex( expected.a, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetAccumulator(), 2 ) << '\n';
        }

        if ( cpu.GetXRegister() != expected.x )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": X mismatch\n";
            std::cerr << "Expected: " << toHex( expected.x, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetXRegister(), 2 ) << '\n';
        }

        if ( cpu.GetYRegister() != expected.y )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": Y mismatch\n";
            std::cerr << "Expected: " << toHex( expected.y, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetYRegister(), 2 ) << '\n';
        }

        if ( cpu.GetStatusRegister() != expected.p )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": P mismatch\n";
            std::cerr << "Expected: " << toHex( expected.p, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetStatusRegister(), 2 ) << '\n';
        }

        if ( cpu.GetStackPointer() != expected.sp )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": SP mismatch\n";
            std::cerr << "Expected: " << toHex( expected.sp, 2 ) << "    "
                      << "Actual: " << toHex( cpu.GetStackPointer(), 2 ) << '\n';
        }

        if ( cpu.GetCycles() != expected.cycles )
        {
            did_fail = true;
            std::cerr << ( line_index + 1 ) << ": Cycles mismatch\n";
            std::cerr << "Expected: " << expected.cycles << "    "
                      << "Actual: " << cpu.GetCycles() << '\n';
        }

        if ( did_fail )
        {
            FAIL() << "Mismatch detected at line " << ( line_index + 1 );
            break;
        }

        cpu.Tick(); // Run one CPU cycle
        line_index++;
    }

    if ( !did_fail )
    {
        std::cout << "Nestest passed.\n";
    }
    else
    {
        std::cerr << "Nestest failed.\n";
    }

    log.close();
}

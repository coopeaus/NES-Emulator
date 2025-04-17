#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"
#include "json.hpp"
#include <cstdint>
#include <fstream>
#include <gtest/gtest.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "paths.h"

using json = nlohmann::json;

// forward declarations
class CpuTest : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
public:
  // All tests assume flat memory model, which is why true is passed to Bus constructor
  Bus  bus;
  CPU &cpu = bus.cpu;
  PPU &ppu = bus.ppu;

  CpuTest() = default;
  void LoadTestCartridge()
  {
    std::string romFile = std::string( paths::roms() ) + "/palette.nes";
    bus.cartridge.LoadRom( romFile );
    bus.cpu.Reset();
  }

  static auto GetJsonData( const std::string &path ) -> json
  // Extracts test cases from a JSON file and returns them as a JSON object, with the
  // help of the nlohmann::json library.
  {
    std::ifstream jsonFile( path );
    if ( !jsonFile.is_open() ) {
      throw std::runtime_error( "Could not open test file: " + path );
    }
    json testCases;
    jsonFile >> testCases;

    if ( !testCases.is_array() ) {
      throw std::runtime_error( "Expected an array of test cases in JSON file" );
    }
    return testCases;
  }

  static void TestStart( const std::string &testName )
  {
    std::cout << '\n';
    std::cout << "---------- " << testName << " Tests ---------" << '\n';
  }
  static void TestEnd( const std::string &testName )
  {
    std::cout << "---------- " << testName << " Tests Complete ---------" << '\n';
    std::cout << '\n';
  }

  void RunTestCase( const json &testCase ) // NOLINT
  {
    // Initialize CPU
    bus.EnableJsonTestMode();
    cpu.EnableJsonTestMode();
    ppu.EnableJsonTestMode();
    cpu.Reset();

    LoadStateFromJson( testCase, "initial" );
    std::string const initialState = GetCPUStateString( testCase, "initial" );
    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.GetProgramCounter(), static_cast<u16>( testCase["initial"]["pc"] ) );
    EXPECT_EQ( cpu.GetAccumulator(), testCase["initial"]["a"] );
    EXPECT_EQ( cpu.GetXRegister(), testCase["initial"]["x"] );
    EXPECT_EQ( cpu.GetYRegister(), testCase["initial"]["y"] );
    EXPECT_EQ( cpu.GetStackPointer(), testCase["initial"]["s"] );
    EXPECT_EQ( cpu.GetStatusRegister(), testCase["initial"]["p"] );

    for ( const auto &ramEntry : testCase["initial"]["ram"] ) {
      uint16_t const address = ramEntry[0];
      uint8_t const  value = ramEntry[1];
      EXPECT_EQ( cpu.Read( address ), value );
    }

    // Fetch, decode, execute
    cpu.DecodeExecute();

    // Check final state
    bool               testFailed = false; // Track if any test has failed
    std::ostringstream errorMessages;      // Accumulate error messages
                                           //
    if ( cpu.GetProgramCounter() != static_cast<u16>( testCase["final"]["pc"] ) ) {
      testFailed = true;
      errorMessages << "PC ";
    }
    if ( cpu.GetAccumulator() != static_cast<u8>( testCase["final"]["a"] ) ) {
      testFailed = true;
      errorMessages << "A ";
    }
    if ( cpu.GetXRegister() != static_cast<u8>( testCase["final"]["x"] ) ) {
      testFailed = true;
      errorMessages << "X ";
    }
    if ( cpu.GetYRegister() != static_cast<u8>( testCase["final"]["y"] ) ) {
      testFailed = true;
      errorMessages << "Y ";
    }
    if ( cpu.GetStackPointer() != static_cast<u8>( testCase["final"]["s"] ) ) {
      testFailed = true;
      errorMessages << "S ";
    }
    if ( cpu.GetStatusRegister() != static_cast<u8>( testCase["final"]["p"] ) ) {
      testFailed = true;
      errorMessages << "P ";
    }
    if ( cpu.GetCycles() != testCase["cycles"].size() ) {
      testFailed = true;
      errorMessages << "Cycle count ";
    }

    for ( const auto &ramEntry : testCase["final"]["ram"] ) {
      uint16_t const address = ramEntry[0];
      uint8_t const  expectedValue = ramEntry[1];
      uint8_t const  actualValue = cpu.Read( address );
      if ( actualValue != expectedValue ) {
        testFailed = true;
        errorMessages << "RAM ";
      }
    }

    std::string const finalState = GetCPUStateString( testCase, "final" );
    // print initial and final state if there are any failures
    if ( testFailed ) {
      std::cout << "Test Case: " << testCase["name"] << '\n';
      std::cout << "Failed: " << errorMessages.str() << '\n';
      std::cout << initialState << '\n';
      std::cout << finalState << '\n';
      std::cout << '\n';
      FAIL();
    }
  }

  void LoadStateFromJson( const json &jsonData, const std::string &state )
  {
    /*
     This function loads the CPU state from json data.
     args:
      jsonData: JSON data returned by GetJsonData
      state: "initial" or "final"
    */
    cpu.SetProgramCounter( jsonData[state]["pc"] );
    cpu.SetAccumulator( jsonData[state]["a"] );
    cpu.SetXRegister( jsonData[state]["x"] );
    cpu.SetYRegister( jsonData[state]["y"] );
    cpu.SetStackPointer( jsonData[state]["s"] );
    cpu.SetStatusRegister( jsonData[state]["p"] );

    // Load memory state from JSON
    for ( const auto &ramEntry : jsonData[state]["ram"] ) {
      uint16_t const address = ramEntry[0];
      uint8_t const  value = ramEntry[1];
      cpu.Write( address, value );
    }
  }

  std::string GetCPUStateString( const json &jsonData, const std::string &state ) const
  {
    /*
    This function provides formatted output for expected vs. actual CPU state values,
    based on provided json data and actual CPU state.
    Args:
      jsonData: JSON data returned by GetJsonData
      state: "initial" or "final"
    */

    // Expected values
    u16 const expectedPc = static_cast<u16>( jsonData[state]["pc"] );
    u8 const  expectedA = jsonData[state]["a"];
    u8 const  expectedX = jsonData[state]["x"];
    u8 const  expectedY = jsonData[state]["y"];
    u8 const  expectedS = jsonData[state]["s"];
    u8 const  expectedP = jsonData[state]["p"];
    u64 const expectedCycles = jsonData["cycles"].size();

    // Actual values
    u16 const actualPc = cpu.GetProgramCounter();
    u8 const  actualA = cpu.GetAccumulator();
    u8 const  actualX = cpu.GetXRegister();
    u8 const  actualY = cpu.GetYRegister();
    u8 const  actualS = cpu.GetStackPointer();
    u8 const  actualP = cpu.GetStatusRegister();
    u64 const actualCycles = cpu.GetCycles();

    // Column Widths
    constexpr int labelWidth = 8;
    constexpr int valueWidth = 14;

    // Use osstringstream to collect output
    std::ostringstream output;

    // Print header
    output << "----------" << state << " State----------" << '\n';
    output << std::left << std::setw( labelWidth ) << "" << std::setw( valueWidth ) << "EXPECTED"
           << std::setw( valueWidth ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto printLine = [&]( const std::string &label, const uint64_t expected, const uint64_t actual ) {
      auto toHexDecimalString = []( const uint64_t value, const int width ) {
        std::stringstream strStream;
        strStream << std::hex << std::uppercase << std::setw( width ) << std::setfill( '0' ) << value << " ("
                  << std::dec << value << ")";
        return strStream.str();
      };

      int width; // NOLINT
      if ( expected > 0xFFFF || actual > 0xFFFF ) {
        width = 8;
      } else if ( expected > 0xFF || actual > 0xFF ) {
        width = 4;
      } else {
        width = 2;
      }

      output << std::left << std::setw( labelWidth ) << label;
      output << std::setw( valueWidth ) << toHexDecimalString( expected, width );
      output << std::setw( valueWidth ) << toHexDecimalString( actual, width ) << '\n';
    };

    // Print registers
    printLine( "pc:", expectedPc, actualPc );
    printLine( "s:", expectedS, actualS );
    printLine( "a:", expectedA, actualA );
    printLine( "x:", expectedX, actualX );
    printLine( "y:", expectedY, actualY );
    printLine( "p:", expectedP, actualP );

    if ( state == "final" ) {
      output << std::left << std::setw( labelWidth ) << "cycles:";
      output << std::setw( valueWidth ) << expectedCycles;
      output << std::setw( valueWidth ) << actualCycles << '\n';
    }

    // Blank line and RAM header
    output << '\n' << "RAM" << '\n';

    // Print RAM entries
    for ( const auto &ramEntry : jsonData[state]["ram"] ) {
      uint16_t const address = ramEntry[0];
      uint8_t const  expectedValue = ramEntry[1];
      uint8_t const  actualValue = cpu.Read( address );

      // Helper lambda to format values as "HEX (DECIMAL)"
      auto formatValue = []( const uint8_t value ) {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' ) << static_cast<int>( value ) << " ("
            << std::dec << static_cast<int>( value ) << ")";
        return oss.str();
      };

      // Format address as hex only (no decimal for addresses)
      std::ostringstream addressStream;
      addressStream << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;

      // Print formatted output
      output << std::left << std::setw( labelWidth ) << addressStream.str();
      output << std::setw( valueWidth ) << formatValue( expectedValue );
      output << std::setw( valueWidth ) << formatValue( actualValue ) << '\n';
    }

    output << "--------------------------------" << '\n';
    output << '\n';

    // Return the accumulated string
    return output.str();
  }
};

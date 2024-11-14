// cpu_test.cpp
#include "bus.h"
#include "cpu.h"
#include "json.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iomanip>
using json = nlohmann::json;

// forward declarations
auto extractTestsFromJson( const std::string &path ) -> json;
void printTestStartMsg( const std::string &testName );
void printTestEndMsg( const std::string &testName );

class CPUTestFixture : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
  protected:
    CPU cpu; // NOLINT
    Bus bus; // NOLINT

    // All tests assume flat memory model, which is why true is passed to Bus constructor
    CPUTestFixture() : cpu( &bus ), bus( true ) {}

    void        RunTestCase( const json &testCase );
    void        LoadStateFromJson( const json &jsonData, const std::string &state );
    std::string GetCPUStateString( const json &jsonData, const std::string &state );
};

// -----------------------------------------------------------------------------
// --------------------------- GENERAL TESTS CASES -----------------------------
//           Put anything here that doesn't neatly fit into a category
// -----------------------------------------------------------------------------
TEST_F( CPUTestFixture, SanityCheck )
{
    // cpu.read and cpu.write shouldn't throw any errors
    u8 test_val = cpu.Read( 0x0000 );
    cpu.Write( 0x0000, test_val );
}

// -----------------------------------------------------------------------------
// --------------------------- ADDRESSING MODE TESTS ---------------------------
// -----------------------------------------------------------------------------
// TODO: Add addressing mode tests here

/* -----------------------------------------------------------------------------
   --------------------------- OPCODE JSON TESTS -------------------------------
                            Tom Harte's json tests.
   -----------------------------------------------------------------------------
*/
// -----------------------------------------------------------------------------
// -------------------------TEST CLASS METHODS ---------------------------------
// -----------------------------------------------------------------------------
void CPUTestFixture::RunTestCase( const json &testCase ) // NOLINT
{
    // Initialize CPU
    // TODO: Reset CPU state

    LoadStateFromJson( testCase, "initial" );
    std::string initial_state = GetCPUStateString( testCase, "initial" );
    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.GetProgramCounter(), u16( testCase["initial"]["pc"] ) );
    EXPECT_EQ( cpu.GetAccumulator(), testCase["initial"]["a"] );
    EXPECT_EQ( cpu.GetXRegister(), testCase["initial"]["x"] );
    EXPECT_EQ( cpu.GetYRegister(), testCase["initial"]["y"] );
    EXPECT_EQ( cpu.GetStackPointer(), testCase["initial"]["s"] );
    EXPECT_EQ( cpu.GetStatusRegister(), testCase["initial"]["p"] );

    for ( const auto &ram_entry : testCase["initial"]["ram"] )
    {
        uint16_t address = ram_entry[0];
        uint8_t  value = ram_entry[1];
        EXPECT_EQ( cpu.Read( address ), value );
    }

    // Temp: print initial state
    std::cout << '\n';
    std::cout << "Loading state from tests/json/small.json" << '\n';
    std::cout << "Test name: " << testCase["name"] << '\n';
    std::cout << initial_state << '\n';

    // TODO: Run CPU fetch-decode-execute method(s) once

    // TODO: Compare the actual final state with the expected final state
}

void CPUTestFixture::LoadStateFromJson( const json &jsonData, const std::string &state )
{
    /*
     This function loads the CPU state from json data.
     args:
      jsonData: JSON data returned by extractTestsFromJson
      state: "initial" or "final"
    */
    cpu.SetProgramCounter( jsonData[state]["pc"] );
    cpu.SetAccumulator( jsonData[state]["a"] );
    cpu.SetXRegister( jsonData[state]["x"] );
    cpu.SetYRegister( jsonData[state]["y"] );
    cpu.SetStackPointer( jsonData[state]["s"] );
    cpu.SetStatusRegister( jsonData[state]["p"] );

    // Load memory state from JSON
    for ( const auto &ram_entry : jsonData[state]["ram"] )
    {
        uint16_t address = ram_entry[0];
        uint8_t  value = ram_entry[1];
        cpu.Write( address, value );
    }
}

std::string CPUTestFixture::GetCPUStateString( const json &jsonData, const std::string &state )
{
    /*
    This function provides formatted output for expected vs. actual CPU state values,
    based on provided json data and actual CPU state.
    Args:
      jsonData: JSON data returned by extractTestsFromJson
      state: "initial" or "final"
    */

    // Expected values
    u16 expected_pc = u16( jsonData[state]["pc"] );
    u8  expected_a = jsonData[state]["a"];
    u8  expected_x = jsonData[state]["x"];
    u8  expected_y = jsonData[state]["y"];
    u8  expected_s = jsonData[state]["s"];
    u8  expected_p = jsonData[state]["p"];
    u64 expected_cycles = jsonData["cycles"].size();

    // Actual values
    u16 actual_pc = cpu.GetProgramCounter();
    u8  actual_a = cpu.GetAccumulator();
    u8  actual_x = cpu.GetXRegister();
    u8  actual_y = cpu.GetYRegister();
    u8  actual_s = cpu.GetStackPointer();
    u8  actual_p = cpu.GetStatusRegister();
    u64 actual_cycles = cpu.GetCycles();

    // Column Widths
    const int label_width = 8;
    const int value_width = 14;

    // Use ostringstream to collect output
    std::ostringstream output;

    // Print header
    output << "----------" << state << " State----------" << '\n';
    output << std::left << std::setw( label_width ) << "" << std::setw( value_width ) << "EXPECTED"
           << std::setw( value_width ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto print_line = [&]( const std::string &label, uint64_t expected, uint64_t actual )
    {
        auto to_hex_decimal_string = []( uint64_t value, int width )
        {
            std::stringstream str_stream;
            str_stream << std::hex << std::uppercase << std::setw( width ) << std::setfill( '0' )
                       << value << " (" << std::dec << value << ")";
            return str_stream.str();
        };

        // Determine hex width based on value
        int width = ( expected > 0xFFFF || actual > 0xFFFF ) ? 8
                    : ( expected > 0xFF || actual > 0xFF )   ? 4
                                                             : 2;

        output << std::left << std::setw( label_width ) << label;
        output << std::setw( value_width ) << to_hex_decimal_string( expected, width );
        output << std::setw( value_width ) << to_hex_decimal_string( actual, width ) << '\n';
    };

    // Print registers
    print_line( "pc:", expected_pc, actual_pc );
    print_line( "s:", expected_s, actual_s );
    print_line( "a:", expected_a, actual_a );
    print_line( "x:", expected_x, actual_x );
    print_line( "y:", expected_y, actual_y );
    print_line( "p:", expected_p, actual_p );

    if ( state == "final" )
    {
        output << std::left << std::setw( label_width ) << "cycles:";
        output << std::setw( value_width ) << expected_cycles;
        output << std::setw( value_width ) << actual_cycles << '\n';
    }

    // Blank line and RAM header
    output << '\n' << "RAM" << '\n';

    // Print RAM entries
    for ( const auto &ram_entry : jsonData[state]["ram"] )
    {
        uint16_t address = ram_entry[0];
        uint8_t  expected_value = ram_entry[1];
        uint8_t  actual_value = cpu.Read( address );

        // Helper lambda to format values as "HEX (DECIMAL)"
        auto format_value = []( uint8_t value )
        {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' )
                << static_cast<int>( value ) << " (" << std::dec << static_cast<int>( value )
                << ")";
            return oss.str();
        };

        // Format address as hex only (no decimal for addresses)
        std::ostringstream address_stream;
        address_stream << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;

        // Print formatted output
        output << std::left << std::setw( label_width ) << address_stream.str();
        output << std::setw( value_width ) << format_value( expected_value );
        output << std::setw( value_width ) << format_value( actual_value ) << '\n';
    }

    output << "--------------------------------" << '\n';
    output << '\n';

    // Return the accumulated string
    return output.str();
}

// -----------------------------------------------------------------------------
// --------------------------- GENERAL HELPERS ---------------------------------
//               Helpers that don't depend on the CPU class
// -----------------------------------------------------------------------------

auto extractTestsFromJson( const std::string &path ) -> json
// Extracts test cases from a JSON file and returns them as a JSON object, with the
// help of the nlohmann::json library.
{
    std::ifstream json_file( path );
    if ( !json_file.is_open() )
    {
        throw std::runtime_error( "Could not open test file: " + path );
    }
    json test_cases;
    json_file >> test_cases;

    if ( !test_cases.is_array() )
    {
        throw std::runtime_error( "Expected an array of test cases in JSON file" );
    }
    return test_cases;
}

void printTestStartMsg( const std::string &testName )
{
    std::cout << '\n';
    std::cout << "---------- " << testName << " Tests ---------" << '\n';
}
void printTestEndMsg( const std::string &testName )
{
    std::cout << "---------- " << testName << " Tests Complete ---------" << '\n';
    std::cout << '\n';
}

// -----------------------------------------------------------------------------
// -------------------------------- MAIN ---------------------------------------
// -----------------------------------------------------------------------------
auto main( int argc, char **argv ) -> int
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

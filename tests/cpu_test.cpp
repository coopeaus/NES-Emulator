// cpu_test.cpp
#include "bus.h"
#include "cpu.h"
#include "json.hpp"
#include <fstream>
#include <gtest/gtest.h>
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

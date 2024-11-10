// cpu_test.cpp
#include "bus.h"
#include "cpu.h"
#include "json.hpp"
#include <fstream>
#include <gtest/gtest.h>
using json = nlohmann::json;
using u8 = uint8_t;
using u16 = uint16_t;
using u64 = uint64_t;

auto extractTestsFromJson( const std::string &path ) -> json;

class CPUTestFixture : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
  protected:
    CPU cpu; // NOLINT
    Bus bus; // NOLINT

    CPUTestFixture() : cpu( &bus ), bus() {}

    void RunTestCase( const json &testCase );
};

auto main( int argc, char **argv ) -> int
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

TEST_F( CPUTestFixture, SanityCheck )
{
    // cpu.read and cpu.write shouldn't throw any errors
    u8 test_val = cpu.Read( 0x0000 );
    cpu.Write( 0x0000, test_val );
}

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

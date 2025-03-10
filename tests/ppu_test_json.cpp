// ppu_test.cpp
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
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

using json = nlohmann::json;

// forward declarations
void printTestStartMsg( const std::string &testName );
void printTestEndMsg( const std::string &testName );
json extractTestsFromJson( const std::string &path );

class PPUTestFixture : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
  protected:
    Bus bus;
    CPU cpu;
    PPU ppu;

    PPUTestFixture() : cpu( bus.cpu ), ppu( bus.ppu ) {}

    void SetUp() override
    {
        // Initialize the PPU and Bus state before each test
        ppu.Reset();
    }

    void                      RunTestCase( const json &testCase );
    void                      LoadStateFromJson( const json &jsonData, const std::string &state );
    [[nodiscard]] std::string GetPPUStateString( const json &jsonData, const std::string &state ) const;

    // Expose private PPU methods
    // PPUTestFixture is a friend class of PPU. To use PPU private methods, we need to create
    // wrappers.
    [[nodiscard]] u8 Read( u16 address );
    void             Write( u16 address, u8 data );
    void             TriggerNmi();
    u16              ResolveNameTableAddress( u16 addr );
    void             UpdateShiftRegisters();
    void             LoadNextBgShiftRegisters();
    void             LoadNametableByte();
    void             LoadAttributeByte();
    void             LoadPatternPlane0Byte();
    void             LoadPatternPlane1Byte();
    void             IncrementScrollX();
    void             IncrementScrollY();
};

/*
################################################################
||                                                            ||
||                     General Test Cases                     ||
||                                                            ||
################################################################
 */

TEST_F( PPUTestFixture, SanityCheck )
{
    // ppu.read and ppu.write shouldn't throw any errors
    u8 const testVal = Read( 0x0000 );
    Write( 0x0000, testVal );
}

// Test Initialization
TEST_F( PPUTestFixture, TestInitialization )
{
    EXPECT_FALSE( ppu.failedPaletteRead ); // Check if palette loading failed
}

/*
################################################################
||                                                            ||
||                   PPU Register Tests                       ||
||                                                            ||
################################################################
*/

/* This is a macro to simplify test creation for json tests
 * The naming convention is <register name>_<operation>_<test description>
 * e.g. PPUCTRL_Write_BasicFunctionality, PPUSTATUS_Read_VerticalBlankClear, etc.
 */
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define PPU_TEST( register_name, operation, description, filename )                                          \
    TEST_F( PPUTestFixture, register_name##_##operation##_##description )                                    \
    {                                                                                                        \
        std::string const testName = #register_name " " #operation " " #description;                         \
        printTestStartMsg( testName );                                                                       \
        json const testCases = extractTestsFromJson( "tests/json/" filename );                               \
        for ( const auto &testCase : testCases ) {                                                           \
            RunTestCase( testCase );                                                                         \
        }                                                                                                    \
        printTestEndMsg( testName );                                                                         \
    }
// NOLINTEND(cppcoreguidelines-macro-usage)

/*
  Testing PPU registers:
  1. Create JSON test files in tests/json
  2. Uncomment the corresponding test below
  3. Build and run the tests
*/

/* Add JSON-based tests here when you create the JSON files */
/* Example: PPU_TEST( PPUCTRL, Write, NmiGeneration, "ppuctrl_nmi.json" ); */

/*
################################################################
||                                                            ||
||                  Direct Register Tests                     ||
||                                                            ||
################################################################
*/

// Test CPU Read Operations
TEST_F( PPUTestFixture, TestHandleCpuRead )
{
    // Test reading from non-readable registers
    EXPECT_EQ( ppu.HandleCpuRead( 0x2000, false ), 0xFF ); // PPUCTRL
    EXPECT_EQ( ppu.HandleCpuRead( 0x2001, false ), 0xFF ); // PPUMASK
    EXPECT_EQ( ppu.HandleCpuRead( 0x2003, false ), 0xFF ); // OAMADDR
    EXPECT_EQ( ppu.HandleCpuRead( 0x2005, false ), 0xFF ); // PPUSCROLL
    EXPECT_EQ( ppu.HandleCpuRead( 0x2006, false ), 0xFF ); // PPUADDR

    // Test reading from PPU Status Register
    // Assuming vertical blank flag is set
    ppu.HandleCpuWrite( 0x2002, 0x00 );                        // Write to clear the status
    EXPECT_EQ( ppu.HandleCpuRead( 0x2002, false ) & 0xE0, 0 ); // Check if vertical blank flag is cleared

    // Test OAM Data Read
    ppu.HandleCpuWrite( 0x2004, 0xAA );             // Write to OAM
    EXPECT_EQ( ppu.HandleCpuRead( 0x2004 ), 0xAA ); // Read back OAM data

    // Test PPU Data Read
    ppu.HandleCpuWrite( 0x2007, 0x55 );             // Write to VRAM
    EXPECT_EQ( ppu.HandleCpuRead( 0x2007 ), 0x55 ); // Read back VRAM data
}

// Test CPU Write Operations
TEST_F( PPUTestFixture, TestHandleCpuWrite )
{
    // Test writing to PPUCTRL
    ppu.HandleCpuWrite( 0x2000, 0x01 );
    // Check if the control register is set correctly
    // You may need to add a method to verify the control register state

    // Test writing to PPUMASK
    ppu.HandleCpuWrite( 0x2001, 0x02 );
    // Verify if rendering is enabled or not based on the mask

    // Test writing to OAMADDR
    ppu.HandleCpuWrite( 0x2003, 0x10 );
    // Verify if the OAM address is set correctly

    // Test writing to OAMDATA
    ppu.HandleCpuWrite( 0x2004, 0xFF );
    // Verify if the OAM data is written correctly

    // Test writing to PPUSCROLL
    ppu.HandleCpuWrite( 0x2005, 0x20 ); // First write
    ppu.HandleCpuWrite( 0x2005, 0x10 ); // Second write
    // Verify the scroll registers are set correctly

    // Test writing to PPUADDR
    ppu.HandleCpuWrite( 0x2006, 0x80 ); // First write
    ppu.HandleCpuWrite( 0x2006, 0x00 ); // Second write
    // Verify the VRAM address is set correctly

    // Test writing to PPU Data
    ppu.HandleCpuWrite( 0x2007, 0xAA );
    // Verify the data is written to VRAM correctly
}

/*
################################################################
||                                                            ||
||                     PPU Core Functions                     ||
||                                                            ||
################################################################
*/

// Test OAM DMA Transfer
TEST_F( PPUTestFixture, TestDmaTransfer )
{
    // Assuming the bus is set up correctly
    bus.cpu.Write( 0x4014, 0x02 ); // Start DMA transfer from 0x0200
    ppu.DmaTransfer( 0x02 );       // Execute the transfer
    // Verify that the OAM contains the expected data
}

// Test Rendering Functionality
TEST_F( PPUTestFixture, TestRendering )
{
    // Simulate a rendering cycle
    for ( int i = 0; i < 340; ++i ) {
        ppu.Tick(); // Advance the PPU cycle
    }
    // Verify the framebuffer or output pixels
}

/*
################################################################
||                                                            ||
||              PPU Internal Helper Functions                 ||
||                                                            ||
################################################################
*/

// Test NameTable Address Resolution
TEST_F( PPUTestFixture, TestNameTableAddressResolution )
{
    // Test with horizontal mirroring
    // Test with vertical mirroring
    // Test with four-screen mirroring
    // Test with single-screen mirroring
}

// Test Background Rendering
TEST_F( PPUTestFixture, TestBackgroundRendering )
{
    // Test pattern table fetching
    // Test attribute table fetching
    // Test palette selection
}

// Test Sprite Rendering
TEST_F( PPUTestFixture, TestSpriteRendering )
{
    // Test sprite evaluation
    // Test sprite fetching
    // Test sprite priority
    // Test sprite zero hit
}

/*
################################################################
||                                                            ||
||                  Implement Helper Methods                  ||
||                                                            ||
################################################################
*/

// Implementation of RunTestCase
void PPUTestFixture::RunTestCase( const json &testCase )
{
    // Initialize PPU
    bus.EnableJsonTestMode(); // If your bus has this method
    ppu.Reset();              // Fixed capitalization

    LoadStateFromJson( testCase, "initial" );
    std::string const initialState = GetPPUStateString( testCase, "initial" );

    // Ensure loaded values match JSON values - add PPU-specific registers
    // Example (adjust based on your PPU's actual registers):
    EXPECT_EQ( ppu.ppuCtrl.value, testCase["initial"]["ppuctrl"] );
    EXPECT_EQ( ppu.ppuMask.value, testCase["initial"]["ppumask"] );
    EXPECT_EQ( ppu.ppuStatus.value, testCase["initial"]["ppustatus"] );
    EXPECT_EQ( ppu.oamAddr, testCase["initial"]["oamaddr"] );
    EXPECT_EQ( ppu.vramAddr.value, testCase["initial"]["vramaddr"] );

    // Check initial VRAM state if specified
    if ( testCase["initial"].contains( "vram" ) ) {
        for ( const auto &vramEntry : testCase["initial"]["vram"] ) {
            u16 const address = vramEntry[0];
            u8 const  value = vramEntry[1];
            EXPECT_EQ( ppu.ReadVRAM( address ), value );
        }
    }

    // Check initial OAM state if specified
    if ( testCase["initial"].contains( "oam" ) ) {
        for ( const auto &oamEntry : testCase["initial"]["oam"] ) {
            u16 const address = oamEntry[0];
            u8 const  value = oamEntry[1];
            EXPECT_EQ( ppu.ReadOAM( address ), value );
        }
    }

    // Perform the operation specified in the test case
    if ( testCase.contains( "operation" ) ) {
        std::string operation = testCase["operation"];
        if ( operation == "cpu_read" ) {
            u16 address = testCase["address"];
            ppu.HandleCpuRead( address, false );
        } else if ( operation == "cpu_write" ) {
            u16 address = testCase["address"];
            u8  data = testCase["data"];
            ppu.HandleCpuWrite( address, data );
        } else if ( operation == "tick" ) {
            int cycles = testCase["cycles"];
            for ( int i = 0; i < cycles; i++ ) {
                ppu.Tick();
            }
        }
        // Add other operations as needed
    }

    // Check final state
    bool               testFailed = false;
    std::ostringstream errorMessages;

    // Check all relevant PPU registers against expected values
    if ( ppu._ppuCtrl.value != static_cast<u8>( testCase["final"]["ppuctrl"] ) ) {
        testFailed = true;
        errorMessages << "PPUCTRL ";
    }
    if ( ppu._ppuMask.value != static_cast<u8>( testCase["final"]["ppumask"] ) ) {
        testFailed = true;
        errorMessages << "PPUMASK ";
    }
    if ( ppu._ppuStatus.value != static_cast<u8>( testCase["final"]["ppustatus"] ) ) {
        testFailed = true;
        errorMessages << "PPUSTATUS ";
    }
    if ( ppu._oamAddr != static_cast<u8>( testCase["final"]["oamaddr"] ) ) {
        testFailed = true;
        errorMessages << "OAMADDR ";
    }
    if ( ppu._vramAddr.value != static_cast<u16>( testCase["final"]["vramaddr"] ) ) {
        testFailed = true;
        errorMessages << "VRAMADDR ";
    }

    // Check final VRAM state
    if ( testCase["final"].contains( "vram" ) ) {
        for ( const auto &vramEntry : testCase["final"]["vram"] ) {
            u16 const address = vramEntry[0];
            u8 const  value = vramEntry[1];
            EXPECT_EQ( ppu.ReadVRAM( address ), value );
        }
    }

    // Check final OAM state
    if ( testCase["final"].contains( "oam" ) ) {
        for ( const auto &oamEntry : testCase["final"]["oam"] ) {
            u16 const address = oamEntry[0];
            u8 const  value = oamEntry[1];
            EXPECT_EQ( ppu.ReadOAM( address ), value );
        }
    }

    std::string const finalState = GetPPUStateString( testCase, "final" );

    // Print initial and final state if there are any failures
    if ( testFailed ) {
        std::cout << "Test Case: " << testCase["name"] << '\n';
        std::cout << "Failed: " << errorMessages.str() << '\n';
        std::cout << initialState << '\n';
        std::cout << finalState << '\n';
        std::cout << '\n';
        FAIL();
    }
}

// Implementation of LoadStateFromJson
void PPUTestFixture::LoadStateFromJson( const json &jsonData, const std::string &state )
{
    // Set PPU registers from JSON data
    ppu._ppuCtrl.value = jsonData[state]["ppuctrl"];
    ppu._ppuMask.value = jsonData[state]["ppumask"];
    ppu._ppuStatus.value = jsonData[state]["ppustatus"];
    ppu._oamAddr = jsonData[state]["oamaddr"];
    ppu._vramAddr.value = jsonData[state]["vramaddr"];

    // You may need to set additional PPU internal state like:
    if ( jsonData[state].contains( "finex" ) ) {
        ppu._fineX = jsonData[state]["finex"];
    }

    if ( jsonData[state].contains( "addrLatch" ) ) {
        ppu._addrLatch = jsonData[state]["addrLatch"];
    }

    // Load OAM memory from JSON if provided
    if ( jsonData[state].contains( "oam" ) ) {
        for ( const auto &oamEntry : jsonData[state]["oam"] ) {
            uint16_t const address = oamEntry[0];
            uint8_t const  value = oamEntry[1];
            ppu.WriteOAM( address, value );
        }
    }

    // Load VRAM state from JSON
    for ( const auto &vramEntry : jsonData[state]["vram"] ) {
        uint16_t const address = vramEntry[0];
        uint8_t const  value = vramEntry[1];
        ppu.WriteVRAM( address, value );
    }
}

// Implementation of GetPPUStateString
std::string PPUTestFixture::GetPPUStateString( const json &jsonData, const std::string &state ) const
{
    // Expected values from JSON
    u8 const  expectedPpuCtrl = jsonData[state]["ppuctrl"];
    u8 const  expectedPpuMask = jsonData[state]["ppumask"];
    u8 const  expectedPpuStatus = jsonData[state]["ppustatus"];
    u8 const  expectedOamAddr = jsonData[state]["oamaddr"];
    u16 const expectedVramAddr = jsonData[state]["vramaddr"];

    // Add other expected values from JSON as needed

    // Actual values from the PPU
    u8 const  actualPpuCtrl = ppu._ppuCtrl.value;
    u8 const  actualPpuMask = ppu._ppuMask.value;
    u8 const  actualPpuStatus = ppu._ppuStatus.value;
    u8 const  actualOamAddr = ppu._oamAddr;
    u16 const actualVramAddr = ppu._vramAddr.value;

    // Column Widths for formatting
    constexpr int labelWidth = 10;
    constexpr int valueWidth = 14;

    // Use ostringstream to collect output
    std::ostringstream output;

    // Print header
    output << "----------" << state << " State----------" << '\n';
    output << std::left << std::setw( labelWidth ) << "" << std::setw( valueWidth ) << "EXPECTED"
           << std::setw( valueWidth ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto printLine = [&]( const std::string &label, const uint64_t expected, const uint64_t actual ) {
        auto toHexDecimalString = []( const uint64_t value, const int width ) {
            std::stringstream strStream;
            strStream << std::hex << std::uppercase << std::setw( width ) << std::setfill( '0' ) << value
                      << " (" << std::dec << value << ")";
            return strStream.str();
        };

        int width;
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

    // Print PPU registers
    printLine( "ppuctrl:", expectedPpuCtrl, actualPpuCtrl );
    printLine( "ppumask:", expectedPpuMask, actualPpuMask );
    printLine( "ppustatus:", expectedPpuStatus, actualPpuStatus );
    printLine( "oamaddr:", expectedOamAddr, actualOamAddr );
    printLine( "vramaddr:", expectedVramAddr, actualVramAddr );

    // Add any additional PPU-specific registers here

    // Print VRAM entries if they exist in the test case
    output << '\n' << "VRAM" << '\n';

    for ( const auto &vramEntry : jsonData[state]["vram"] ) {
        uint16_t const address = vramEntry[0];
        uint8_t const  expectedValue = vramEntry[1];
        uint8_t const  actualValue = ppu.ReadVRAM( address );

        // Helper lambda to format values as "HEX (DECIMAL)"
        auto formatValue = []( const uint8_t value ) {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' )
                << static_cast<int>( value ) << " (" << std::dec << static_cast<int>( value ) << ")";
            return oss.str();
        };

        // Format address as hex only
        std::ostringstream addressStream;
        addressStream << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;

        // Print formatted output
        output << std::left << std::setw( labelWidth ) << addressStream.str();
        output << std::setw( valueWidth ) << formatValue( expectedValue );
        output << std::setw( valueWidth ) << formatValue( actualValue ) << '\n';
    }

    output << "--------------------------------" << '\n';
    output << '\n';

    return output.str();
}

/*
########################################
||    Expose private methods here     ||
########################################
*/

// Read wrapper
u8 PPUTestFixture::Read( const u16 address )
{
    return ppu.Read( address );
}

// Write wrapper
void PPUTestFixture::Write( const u16 address, const u8 data )
{
    ppu.Write( address, data );
}

void PPUTestFixture::TriggerNmi()
{
    ppu.TriggerNmi();
}

u16 PPUTestFixture::ResolveNameTableAddress( u16 addr )
{
    return ppu.ResolveNameTableAddress( addr );
}

void PPUTestFixture::UpdateShiftRegisters()
{
    ppu.UpdateShiftRegisters();
}

void PPUTestFixture::LoadNextBgShiftRegisters()
{
    ppu.LoadNextBgShiftRegisters();
}

void PPUTestFixture::LoadNametableByte()
{
    ppu.LoadNametableByte();
}

void PPUTestFixture::LoadAttributeByte()
{
    ppu.LoadAttributeByte();
}

void PPUTestFixture::LoadPatternPlane0Byte()
{
    ppu.LoadPatternPlane0Byte();
}

void PPUTestFixture::LoadPatternPlane1Byte()
{
    ppu.LoadPatternPlane1Byte();
}

void PPUTestFixture::IncrementScrollX()
{
    ppu.IncrementScrollX();
}

void PPUTestFixture::IncrementScrollY()
{
    ppu.IncrementScrollY();
}

/*
################################################################
||                                                            ||
||                  General Helper Functions                  ||
||                                                            ||
################################################################
*/

// Extract tests from JSON file
json extractTestsFromJson( const std::string &path )
{
    std::ifstream file( path );
    if ( !file ) {
        throw std::runtime_error( "Failed to open file: " + path );
    }

    json jsonData;
    file >> jsonData;
    return jsonData;
}

// Print test start message
void printTestStartMsg( const std::string &testName )
{
    std::cout << "\n\nTesting " << testName << "...\n";
    std::cout << std::string( 80, '-' ) << std::endl;
}

// Print test end message
void printTestEndMsg( const std::string &testName )
{
    std::cout << std::string( 80, '-' ) << std::endl;
    std::cout << "Finished testing " << testName << "\n";
}

// -----------------------------------------------------------------------------
// -------------------------------- MAIN ---------------------------------------
// -----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

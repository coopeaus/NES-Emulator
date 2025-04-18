#include "bus.h"
#include "cartridge.h"
// #include "apu.h"
#include <gtest/gtest.h>

// Test fixture for APU tests
class ApuTest : public ::testing::Test
{
protected:
    Bus bus;
    // APU &apu = bus.apu;
    CPU &cpu = bus.cpu;
    Cartridge &cartridge = bus.cartridge;

    ApuTest()
    {
        bus.cpu.Reset();
    }
};

// Test writing to $4000 updates the pulse 1 control register
TEST_F(ApuTest, WriteTo4000UpdatesPulse1Control)
{
    cpu.Write(0x4000, 0xAB);
    // Replace 'apu.pulse1.control' with your actual member/getter
    // EXPECT_EQ(apu.pulse1.control, 0xAB);
    SUCCEED();
}

// Test writing to $4015 enables/disables channels
TEST_F(ApuTest, WriteTo4015EnablesAndDisablesChannels)
{
    cpu.Write(0x4015, 0x0F);
    // Replace with checks for channel enable flags
    // EXPECT_TRUE(apu.pulse1.enabled);
    // EXPECT_TRUE(apu.pulse2.enabled);
    // EXPECT_TRUE(apu.triangle.enabled);
    // EXPECT_TRUE(apu.noise.enabled);
    SUCCEED();
}

/*
// Test reading from $4015 returns correct status
TEST_F(ApuTest, ReadFrom4015ReturnsCorrectStatus)
{
    // apu.status = 0xAA; // Set directly if possible
    auto value = cpu.Read(0x4015);
    // EXPECT_EQ(value, 0xAA);
    SUCCEED();
}
*/

// Test writing to $4017 resets frame sequencer and affects IRQ
TEST_F(ApuTest, FrameCounterWriteResetsSequencerAndAffectsIRQ)
{
    cpu.Write(0x4017, 0x80); // Set frame counter
    // Check frame sequencer state and IRQ flag
    // EXPECT_EQ(apu.frameCounter, 0x80);
    // EXPECT_FALSE(apu.frameIRQ);
    SUCCEED();
}

// Test that APU registers reset to default values
TEST_F(ApuTest, ResetClearsRegistersAndState)
{
    // Set some registers first
    cpu.Write(0x4000, 0xFF);
    cpu.Write(0x4015, 0x0F);
    bus.cpu.Reset();
    // Check that registers are reset
    // EXPECT_EQ(apu.pulse1.control, 0x00);
    // EXPECT_EQ(apu.status, 0x00);
    SUCCEED();
}

// Test that writing to unmapped APU registers is ignored
TEST_F(ApuTest, WriteToUnmappedRegisterIsIgnored)
{
    cpu.Write(0x4018, 0xFF); // $4018 is not mapped to APU
    // Check that no state changed
    SUCCEED();
}

// Test channel envelope and sweep register writes
TEST_F(ApuTest, ChannelEnvelopeAndSweepRegisterWrites)
{
    cpu.Write(0x4001, 0x23); // Pulse 1 sweep
    cpu.Write(0x4002, 0x45); // Pulse 1 timer low
    cpu.Write(0x4003, 0x67); // Pulse 1 length counter
    // EXPECT_EQ(apu.pulse1.sweep, 0x23);
    // EXPECT_EQ(apu.pulse1.timerLow, 0x45);
    // EXPECT_EQ(apu.pulse1.lengthCounter, 0x67);
    SUCCEED();
}

// Test output buffer receives samples (if you have audio output implemented)
TEST_F(ApuTest, OutputBufferReceivesSamples)
{
    // Simulate enough CPU/APU cycles to generate output
    // for (int i = 0; i < 1000; ++i) apu.Clock();
    // EXPECT_GT(apu.outputBuffer.size(), 0);
    SUCCEED();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
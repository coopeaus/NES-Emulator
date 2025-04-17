// Google Test Basics: TEST and TEST_F
//
// - `TEST`: Defines a standalone test case.
// - `TEST_F`: Defines a test case that uses a fixture (a shared setup/teardown
// environment).
//
// Common Assertions:
// - EXPECT_EQ(val1, val2): Checks if val1 == val2 (non-fatal, continues test on
// failure).
// - ASSERT_EQ(val1, val2): Checks if val1 == val2 (fatal, stops test on
// failure).
// - EXPECT_NE, EXPECT_LT, EXPECT_GT, EXPECT_LE, EXPECT_GE: Comparison macros.
// - EXPECT_TRUE(condition) / EXPECT_FALSE(condition): Checks a boolean
// condition.
//
// Basic Usage:
// 1. Include <gtest/gtest.h>.
// 2. Define tests using `TEST` or `TEST_F`.
// 3. Compile using settings from CMakeLists.txt and run with `ctest`.
//
// Below is an example of both `TEST` and `TEST_F` usage.

#include <gtest/gtest.h>

class Counter
{
  public:
    Counter() = default;

    void              increment() { ++_count; }
    void              decrement() { --_count; }
    void              reset() { _count = 0; }
    [[nodiscard]] int getCount() const { return _count; }

  private:
    int _count{};
};

// === TEST ===
// Simple, standalone tests without any shared setup
TEST( CounterTests, Increment )
{
    Counter counter;
    counter.increment();
    EXPECT_EQ( counter.getCount(), 1 );
    counter.increment();
    EXPECT_EQ( counter.getCount(), 2 );
}

TEST( CounterTests, Reset )
{
    Counter counter;
    counter.increment();
    counter.reset();
    EXPECT_EQ( counter.getCount(), 0 );
}

// === Test Fixture with TEST_F ===
// Shared setup for tests using a fixture
class CounterTest : public ::testing::Test
{
  protected:
    Counter counter; // Shared instance for all tests in this fixture

    void SetUp() override
    {
        counter.reset(); // Ensure counter starts from zero
    }

    void TearDown() override
    {
        // Optional cleanup after each test, if needed
    }
};

// Tests using the shared Counter instance from the fixture
TEST_F( CounterTest, MultipleIncrements )
{
    counter.increment();
    counter.increment();
    EXPECT_EQ( counter.getCount(), 2 );
}

TEST_F( CounterTest, Decrement )
{
    counter.increment();
    counter.increment();
    counter.decrement();
    EXPECT_EQ( counter.getCount(), 1 );
}

TEST_F( CounterTest, NegativeCount )
{
    counter.decrement();
    EXPECT_EQ( counter.getCount(), -1 );
}

// main function to run all tests
int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

int test_function()
{
    return 0;
}

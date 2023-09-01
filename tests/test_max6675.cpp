#include <gtest/gtest.h>

extern "C" {
#include "max6675.h"
}

class TestMax6675 : public testing::Test {
   protected:
    float temperature;
    uint16_t reading;

    void SetUp() override {
        temperature = 0;
        reading = 0;
    }
};

TEST_F(TestMax6675, InvalidDummy) {
    reading = (4 << 3) | (1 << 15);
    ASSERT_EQ(1, max6675_process(reading, &temperature));
}

TEST_F(TestMax6675, OpenThermoCoupleInput) {
    reading = (12 << 3) | (2 << 1);
    ASSERT_EQ(1, max6675_process(reading, &temperature));
}

TEST_F(TestMax6675, BadDeviceId) {
    reading = (1 << 1);
    ASSERT_EQ(1, max6675_process(reading, &temperature));
}

TEST_F(TestMax6675, ValidReading) {
    reading = (24 << 3);

    ASSERT_EQ(0, max6675_process(reading, &temperature));
    ASSERT_EQ(6, temperature);
}

TEST_F(TestMax6675, ValidFloatReading) {
    reading = (25 << 3);

    ASSERT_EQ(0, max6675_process(reading, &temperature));
    ASSERT_EQ(6.25, temperature);
}

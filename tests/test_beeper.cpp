#include <gtest/gtest.h>

extern "C" {
#include "beeper.h"
}

class TestBeeper : public testing::Test {
   protected:
    struct BeeperConfig config;
    struct BeeperConfig result;

    void SetUp() override {
        config = {0};
        result = {0};
    }
};

TEST_F(TestBeeper, ParseDump) {
    config.beeps = 15;
    config.msOn = 500;
    config.msOff = 200;

    uint32_t value = beeper_dump(&config);
    beeper_parse(value, &result);

    ASSERT_EQ(15, result.beeps);
    ASSERT_EQ(500, result.msOn);
    ASSERT_EQ(200, result.msOff);
}

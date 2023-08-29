#include <gtest/gtest.h>

extern "C" {
#include "uid.h"
}

TEST(TestUid, UidToStr) {
    char uid_str[17];

    uint8_t uid[8] = {0x7A, 0x12, 0x78, 0x89, 0xFE, 0x7A, 0x0B, 0xE0};
    std::fill_n(uid_str, 17, 1);

    uid_to_str(uid, uid_str);
    ASSERT_EQ("e00b7afe8978127a", std::string(uid_str));

    uint8_t uid2[8] = {0x00, 0x67, 0xA4, 0x9F, 0x0F, 0x0E, 0xaa, 0x00};
    std::fill_n(uid_str, 17, 0xFF);

    uid_to_str(uid2, uid_str);
    ASSERT_EQ("00aa0e0f9fa46700", std::string(uid_str));
}

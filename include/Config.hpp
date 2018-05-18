#pragma once

#include <cstdint>
#include <array>

namespace ic::config
{
    constexpr uint16_t DEFAULT_PORT = 15317;
    constexpr uint8_t ISENCOIN_VERSION = 1;
    constexpr uint8_t ISENCOIN_REWARD = 10;
    constexpr std::array<unsigned char, 32> ISENCOIN_NULL_ADDRESS = {};
    namespace tx_field_chars
    {
        constexpr char sender = char(0x3E);
        constexpr char receiver = char(0x3C);
        constexpr char amount = char(0x24);
        constexpr char timestamp = char(0x2E);
    }
}

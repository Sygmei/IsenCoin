#pragma once

#include <cstdint>
#include <array>

namespace ic::config
{
    constexpr uint16_t DEFAULT_PORT = 15317;
    constexpr uint8_t ISENCOIN_VERSION = 1;
    constexpr float ISENCOIN_REWARD = 10;
    constexpr uint8_t ISENCOIN_DIFFICULTY = 5;
    constexpr uint8_t DEFAULT_THREADS = 1;
#ifdef _WIN32
    constexpr const char* DEFAULT_BIND = "";
#else
    constexpr const char* DEFAULT_BIND = "localhost";
#endif
    constexpr std::array<unsigned char, 32> ISENCOIN_NULL_ADDRESS = {
        0x3b, 0x6a, 0x27, 0xbc, 0xce, 0xb6, 0xa4, 0x2d, 0x62, 0xa3, 0xa8,
        0xd0, 0x2a, 0x6f, 0x0d, 0x73, 0x65, 0x32, 0x15, 0x77, 0x1d, 0xe2,
        0x43, 0xa6, 0x3a, 0xc0, 0x48, 0xa1, 0x8b, 0x59, 0xda, 0x29 };
    constexpr std::array<unsigned char, 64> ISENCOIN_NULL_PV_ADDRESS = { 
        0x50, 0x46, 0xad, 0xc1, 0xdb, 0xa8, 0x38, 0x86, 0x7b, 0x2b, 0xbb, 
        0xfd, 0xd0, 0xc3, 0x42, 0x3e, 0x58, 0xb5, 0x79, 0x70, 0xb5, 0x26, 
        0x7a, 0x90, 0xf5, 0x79, 0x60, 0x92, 0x4a, 0x87, 0xf1, 0x56, 0x0a, 
        0x6a, 0x85, 0xea, 0xa6, 0x42, 0xda, 0xc8, 0x35, 0x42, 0x4b, 0x5d, 
        0x7c, 0x8d, 0x63, 0x7c, 0x00, 0x40, 0x8c, 0x7a, 0x73, 0xda, 0x67, 
        0x2b, 0x7f, 0x49, 0x85, 0x21, 0x42, 0x0b, 0x6d, 0xd3 };
    namespace tx_field_chars
    {
        constexpr char sender = char(0x3E);
        constexpr char receiver = char(0x3C);
        constexpr char amount = char(0x24);
        constexpr char timestamp = char(0x2E);
    }
}

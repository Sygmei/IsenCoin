#pragma once

namespace ic
{
    using private_key_t = std::array<unsigned char, 64>;
    using public_key_t = std::array<unsigned char, 32>;
    using block_hash_t = std::array<unsigned char, 144>;
    using signature_t = std::array<unsigned char, 64>;
    using amount_t = float;
    using timestamp_t = uint64_t;
}
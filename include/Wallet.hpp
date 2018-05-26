#pragma once

#include <array>

#include <Types.hpp>

namespace ic
{
    class Wallet
    {
    private:
        private_key_t m_private_key;
        public_key_t m_public_key;
    public:
        Wallet() = default;
        Wallet(const Wallet& wallet);
        Wallet(std::array<unsigned char, 64>, std::array<unsigned char, 32> public_key);
        Wallet(unsigned char const (&private_key)[64], unsigned char const (&public_key)[32]);

        static double Benchmark();
        void generate(std::string prefix = "");
        
        private_key_t get_private_key() const;
        public_key_t get_public_key() const;
        std::string get_b58_public_key() const;
        std::string get_b58_private_key() const;

        float get_funds();

        void send(const Wallet& wallet, float amount);
        operator public_key_t() const;
    };
}
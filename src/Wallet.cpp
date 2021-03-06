#include <Wallet.hpp>

#include <base58/base58.hpp>
#include <ed25519/ed25519.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <numeric>
#include "Chain.hpp"

namespace ic 
{
    void Wallet::generate(std::string prefix)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, std::numeric_limits<unsigned char>::max());
        constexpr unsigned int SEED_SIZE = 32;

        unsigned char seed[SEED_SIZE];
        unsigned char public_key[32];
        unsigned char private_key[64];

        do
        {
            for (unsigned char& i : seed)
                i = dis(gen); //lol
            ed25519_create_keypair(public_key, private_key, seed);

            std::copy(std::begin(private_key), std::end(private_key), std::begin(m_private_key));
            std::copy(std::begin(public_key), std::end(public_key), std::begin(m_public_key));
        } while (this->get_b58_public_key().rfind(prefix, 0) != 0);
        Log->debug("Generated valid address : ", this->get_b58_public_key());
    }

    Wallet::Wallet(const Wallet& wallet)
    {
        m_public_key = wallet.m_public_key;
        m_private_key = wallet.m_private_key;
    }

    Wallet::Wallet(std::array<unsigned char, 64> private_key, std::array<unsigned char, 32> public_key)
    {
        m_private_key = private_key;
        m_public_key = public_key;
    }

    Wallet::Wallet(unsigned char const (&private_key)[64], unsigned char const (&public_key)[32])
    {
        std::copy(std::begin(private_key), std::end(private_key), std::begin(m_private_key));
        std::copy(std::begin(public_key), std::end(public_key), std::begin(m_public_key));
    }

    double Wallet::Benchmark()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, std::numeric_limits<unsigned char>::max());
        constexpr unsigned int SEED_SIZE = 32;

        unsigned char seed[SEED_SIZE];
        unsigned char public_key[32];
        unsigned char private_key[64];

        std::string encoded;
        std::vector<double> time_vector;
        for (unsigned int i = 0; i < 10000; i++)
        {
            const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (unsigned char& i : seed)
                i = dis(gen);
            ed25519_create_keypair(public_key, private_key, seed);

            std::string str_pub(std::begin(public_key), std::end(public_key));
            //encoded = Base58::base58().encode(str_pub);
            const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            time_vector.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
        }
        const double average = std::accumulate(time_vector.begin(), time_vector.end(), 0.0) / double(time_vector.size());
        return average;
    }

    private_key_t Wallet::get_private_key() const
    {
        return m_private_key;
    }

    public_key_t Wallet::get_public_key() const
    {
        return m_public_key;
    }

    std::string Wallet::get_b58_public_key() const
    {
        return base58::encode(m_public_key.data(), m_public_key.data() + m_public_key.size());
    }

    std::string Wallet::get_b58_private_key() const
    {
        return base58::encode(m_private_key.data(), m_private_key.data() + m_private_key.size());
    }

    float Wallet::get_funds()
    {
        float funds = 0;
        for (const auto& blk : Chain::Blockchain().get_blocks())
        {
            for (const auto& tx : blk->get_transactions())
            {
                /*Log->warn("Testing {}", tx->as_string());
                if (tx->is_reward())
                    Log->error("PAY ATTENTION ======================+>0");*/
                if (tx->get_sender() == m_public_key)
                {
                    //Log->warn("Remove tx amount {}", tx->as_string());
                    funds -= tx->get_amount();
                }
                else if (tx->get_receiver() == m_public_key)
                {
                    //Log->warn("Add tx amount {}", tx->as_string());
                    funds += tx->get_amount();
                }
            }
        }
        return funds;
    }

    Wallet::operator public_key_t() const
    {
        return m_public_key;
    }
}

#include <Wallet.hpp>

#include <base_x/base_x.hh>
#include <ed25519/ed25519.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

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

        std::string encoded;
        do
        {
            for (unsigned int i = 0; i < SEED_SIZE; i++)
            {
                seed[i] = dis(gen);
            }
            ed25519_create_keypair(public_key, private_key, seed);

            std::copy(std::begin(private_key), std::end(private_key), std::begin(m_private_key));
            std::copy(std::begin(public_key), std::end(public_key), std::begin(m_public_key));

            std::string str_pub(std::begin(public_key), std::end(public_key));
            encoded = Base58::base58().encode(str_pub);
        } while (encoded.rfind(prefix, 0) != 0);
        std::cout << "FOUND ADDRESS : " << encoded << std::endl;
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

    Wallet::Wallet(bool generate)
    {
        if (generate)
            this->generate();
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
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (unsigned int i = 0; i < SEED_SIZE; i++)
            {
                seed[i] = dis(gen);
            }
            ed25519_create_keypair(public_key, private_key, seed);

            std::string str_pub(std::begin(public_key), std::end(public_key));
            encoded = Base58::base58().encode(str_pub);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            time_vector.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
        }
        double average = std::accumulate(time_vector.begin(), time_vector.end(), 0.0) / double(time_vector.size());
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

    Wallet::operator public_key_t()
    {
        return m_public_key;
    }
}
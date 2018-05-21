#include <Block.hpp>
#include <Config.hpp>
#include <picosha2.hpp>

#include <ed25519/sha512.h>
#include "base58/base58.hpp"
#include "Utils.hpp"

namespace ic 
{
    bool Block::is_nonce_valid(unsigned long long int nonce)
    {
        signature_t current_hash = get_hash(nonce);
        for (unsigned int i = 0; i < std::ceil(float(config::ISENCOIN_DIFFICULTY) / 2.0); i++)
        {
            //Log->trace("current_hash[{}] = {}", i, current_hash[i]);
            const uint8_t& current_value = current_hash[i];
            if ((config::ISENCOIN_DIFFICULTY % 2) && i == (config::ISENCOIN_DIFFICULTY / 2))
            {
                Log->warn("[{}, {}, {}, {}], i = {}, c_v = {}", current_hash[0], current_hash[1], current_hash[2], current_hash[3], i, (current_value >> 4));
                return (current_value >> 4) == 0;
            }
            else if (current_value)
                return false;
        }
        Log->warn("COMPLETE LEADING ZEROS FOUND");
        return true;
    }

    void Block::calculate_merkle_root()
    {
        std::vector<signature_t> transactions_signatures;
        for (const Transaction& tx : m_transactions)
            transactions_signatures.push_back(tx.get_signature());
        signature_t temp_merkle_root = Transaction::get_merkel_root(transactions_signatures);
        std::copy(temp_merkle_root.begin(), temp_merkle_root.end(), merkle_root.data());
    }

    Block::Block(std::vector<Transaction> transactions)
    {
        m_transactions = transactions;
        calculate_merkle_root();
    }

    void Block::validate()
    {
    }

    signature_t Block::get_hash(unsigned long long int nonce)
    {
        uint32_t* nonce_slot = reinterpret_cast<uint32_t*>(merkle_root.data() + sizeof(signature_t));
        *nonce_slot = nonce;
        signature_t final_hash;
        sha512(merkle_root.data(), merkle_root.size(), final_hash.data());
        return final_hash;
    }

    signature_t Block::get_hash()
    {
        return get_hash(m_nonce);
    }

    void Block::mine(uint8_t threads)
    {
        std::vector<std::thread> thread_pool;
        Log->debug("Starting mining");
        for (uint8_t thread_index = 0; thread_index < threads; thread_index++)
        {
            thread_pool.emplace_back([&](uint8_t starting_index, uint8_t increment_level)
            {
                unsigned long long nonce = starting_index;
                while (!m_validated && !is_nonce_valid(nonce))
                {
                    nonce += increment_level;
                    if ((nonce % 1000000) == 0)
                        Log->debug("Nonce : {}", nonce);
                }
                Log->warn("THREAD {} RESULT", starting_index);
                Log->debug("Nonce : {}", nonce);
                signature_t f_hash = get_hash(nonce);
                Log->warn("Resulting in following hash : {}", char_array_to_hex(f_hash));
                if (is_nonce_valid(nonce))
                    m_nonce = nonce;
                m_validated = true;
            }, thread_index, threads);
        }
        for (std::thread &t : thread_pool) {
            if (t.joinable()) {
                t.join();
            }
        }

        Log->warn("Mining ended with nonce {}", m_nonce);
        const signature_t f_hash = get_hash(m_nonce);
        Log->error("Resulting in following hash : {}", char_array_to_hex(f_hash));
    }
}


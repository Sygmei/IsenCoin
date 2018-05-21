#include <Block.hpp>
#include <Config.hpp>
#include <picosha2.hpp>

#include <ed25519/sha512.h>
#include "base58/base58.hpp"
#include "Utils.hpp"
#include "Exceptions.hpp"

namespace ic 
{
    bool Block::is_nonce_valid(block_hash_t& block_hash, uint64_t nonce, timestamp_t timestamp)
    {
        signature_t current_hash = Block::get_hash(block_hash, nonce, timestamp);
        for (unsigned int i = 0; i < std::ceil(float(config::ISENCOIN_DIFFICULTY) / 2.0); i++)
        {
            //Log->trace("current_hash[{}] = {}", i, current_hash[i]);
            const uint8_t& current_value = current_hash[i];
            if ((config::ISENCOIN_DIFFICULTY % 2) && i == (config::ISENCOIN_DIFFICULTY / 2))
            {
                Log->warn("[{}, {}, {}, {}], i = {}, c_v = {}", current_hash[0], current_hash[1], current_hash[2], current_hash[3], i, (current_value >> 4));
                return (current_value >> 4) == 0;
            }
            if (current_value)
                return false;
        }
        return true;
    }

    signature_t Block::calculate_merkle_root()
    {
        if (!m_transactions.empty())
        {
            std::vector<signature_t> transactions_signatures;
            for (const Transaction& tx : m_transactions)
                transactions_signatures.push_back(tx.get_signature());
            return Transaction::get_merkel_root(transactions_signatures);
        }
        else
            return {};
    }

    block_hash_t Block::fill_block_hash()
    {
        block_hash_t block_hash;
        std::copy(m_previous_hash.begin(), m_previous_hash.end(), block_hash.begin());
        signature_t temp_merkle_root = calculate_merkle_root();
        std::copy(temp_merkle_root.begin(), temp_merkle_root.end(), block_hash.data() + sizeof(signature_t));
        return block_hash;
    }

    Block::Block()
    {
        m_previous_hash = {};
        m_current_hash = {};
    }

    Block::Block(Block& previous_block, std::vector<Transaction> transactions)
    {
        m_previous_hash = previous_block.get_hash();
        m_transactions = transactions;
        fill_block_hash();
    }

    Block::Block(signature_t previous_hash, std::vector<Transaction> transactions, uint64_t nonce,
        timestamp_t timestamp)
    {
        m_previous_hash = previous_hash;
        m_transactions = transactions;
        m_nonce = nonce;
        m_timestamp = timestamp;
        validate();
    }

    void Block::validate()
    {
        if (is_nonce_valid(m_current_hash, m_nonce, m_timestamp))
            m_validated = true;
        else
            throw except::InvalidBlockException();
    }

    signature_t Block::get_hash(block_hash_t& block_hash, uint64_t nonce, timestamp_t timestamp)
    {
        timestamp_t* timestamp_slot = reinterpret_cast<timestamp_t*>(block_hash.data() + 2 * sizeof(signature_t));
        *timestamp_slot = timestamp;
        uint64_t* nonce_slot = reinterpret_cast<uint64_t*>(block_hash.data() + 2 * sizeof(signature_t) + sizeof(timestamp_t));
        *nonce_slot = nonce;
        signature_t final_hash;
        sha512(block_hash.data(), block_hash.size(), final_hash.data());
        return final_hash;
    }

    signature_t Block::get_hash()
    {
        if (m_validated)
            return Block::get_hash(m_current_hash, m_nonce, m_timestamp);
        else
            throw except::InvalidBlockException();
    }

    void Block::mine(uint8_t threads)
    {
        std::vector<std::thread> thread_pool;
        Log->debug("Starting mining");
        block_hash_t thread_base_hash = fill_block_hash();
        for (uint8_t thread_index = 0; thread_index < threads; thread_index++)
        {
            thread_pool.emplace_back([&](block_hash_t thread_hash, uint8_t starting_index, uint8_t increment_level)
            {
                unsigned long long nonce = starting_index;
                
                std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
                timestamp_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
                while (!m_validated && !is_nonce_valid(thread_hash, nonce, timestamp))
                {
                    tp = std::chrono::system_clock::now();
                    timestamp = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
                    nonce += increment_level;
                    if ((nonce % 1000000) == 0)
                        Log->debug("Nonce : {} = {}", nonce, char_array_to_hex(thread_hash));
                }
                if (!m_validated)
                {
                    m_nonce = nonce;
                    m_current_hash = thread_hash;
                    m_timestamp = timestamp;
                }
                m_validated = true;
            }, thread_base_hash, thread_index, threads);
        }
        for (std::thread &t : thread_pool) {
            if (t.joinable()) {
                t.join();
            }
        }

        Log->warn("Mining ended with nonce {}", m_nonce);
        const signature_t f_hash = get_hash();
        Log->error("Resulting in following hash : {}", char_array_to_hex(f_hash));
    }
}


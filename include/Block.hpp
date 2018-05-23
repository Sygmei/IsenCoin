#pragma once

#include <vector>

#include <Transaction.hpp>

namespace ic
{
    class Block 
    {
    private:
        std::vector<Transaction> m_transactions;
        int depth = 0;
        uint64_t m_nonce = 0;
        signature_t m_previous_hash;
        block_hash_t m_current_hash;
        timestamp_t m_timestamp;
        bool m_validated = false;
        bool m_interrupt = false;
        bool is_nonce_valid(block_hash_t& block_hash, uint64_t nonce);
        signature_t calculate_merkle_root();
        block_hash_t fill_block_hash();
        void generate_timestamp();
    public:
        Block();
        Block(Block& previous_block, std::vector<Transaction> transactions = {});
        Block(signature_t previous_hash, std::vector<Transaction> transactions = {}, uint64_t nonce = 0, timestamp_t timestamp = 0);
        void add_transaction(const Transaction& tx);
        void validate();
        signature_t get_hash(block_hash_t& block_hash, uint64_t nonce);
        signature_t get_hash();
        uint64_t get_nonce();
        void mine(uint8_t threads);
        bool is_valid() const;
    };
}
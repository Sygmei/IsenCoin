#pragma once

#include <vector>

#include <Config.hpp>
#include <Transaction.hpp>

namespace ic
{
    class Block 
    {
    private:
        std::vector<std::unique_ptr<Transaction>> m_transactions;
        uint64_t m_depth = 0;
        uint64_t m_nonce = 0;
        signature_t m_previous_hash;
        block_hash_t m_current_hash;
        timestamp_t m_timestamp;
        bool m_validated = false;
        bool m_mining = false;
        bool is_nonce_valid(block_hash_t& block_hash, uint64_t nonce);
        signature_t calculate_merkle_root();
        block_hash_t fill_block_hash();
        void generate_timestamp();
    public:
        Block();
		Block(const Block& block);
        Block(uint64_t index, signature_t previous_block, std::vector<Transaction> transactions = {});
        Block(uint64_t index, signature_t previous_hash, uint64_t nonce, timestamp_t timestamp, std::vector<Transaction> transactions = {});
        Block& operator=(const Block& block);
        void add_transaction(const Transaction& tx);
        void validate();
        signature_t get_hash(block_hash_t& block_hash, uint64_t nonce) const;
        signature_t get_hash();
        std::string get_hex_hash();
        uint64_t get_nonce();
        void mine(uint8_t threads, public_key_t reward_recv = config::ISENCOIN_NULL_ADDRESS);
        bool is_valid() const;
        bool is_mining() const;
        unsigned int get_tx_amount() const;
        std::vector<Transaction*> get_transactions() const;
        unsigned int get_depth() const;
        void clean_block();
    };
}
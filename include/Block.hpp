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
        unsigned long long int m_nonce = 0;
        signature_t m_previous_hash;
        signature_and_nonce_t merkle_root;
        int m_time = 0;
        bool m_validated = false;
        bool is_nonce_valid(unsigned long long int nonce);
        void calculate_merkle_root();
    public:
        Block(std::vector<Transaction> transactions);
        void validate();
        signature_t get_hash(unsigned long long int nonce);
        signature_t get_hash();
        void mine(uint8_t threads);
    };
}
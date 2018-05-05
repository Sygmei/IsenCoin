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
        int nonce = 0;
        int m_current_hash = 0;
        int m_previous_hash = 0;
        int m_time = 0;    
    public:
        void validate();
    };
}
#pragma once

#include <Block.hpp>
#include <Transaction.hpp>

namespace ic
{
    class Chain 
    {
    private:
        std::vector<Transaction> m_transactions;
        std::vector<Block> m_blocks;
    public:
        Wallet& create_wallet();
    };
}
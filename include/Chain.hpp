#pragma once

#include <memory>

#include <Block.hpp>
#include <Transaction.hpp>
#include <Wallet.hpp>

namespace ic
{
    class Chain 
    {
    private:
        Block m_current_block;
        std::vector<std::unique_ptr<Block>> m_blocks;
    public:
        Chain();
        Block& get_current_block();
        Block& get_block_at_index(unsigned int index);
        Block& create_new_block();
    };

    extern Chain Blockchain;
}
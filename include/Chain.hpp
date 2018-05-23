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
        static std::unique_ptr<Chain> s_blockchain;
        Chain();
        friend std::unique_ptr<Chain> std::make_unique<Chain>();
    public:
        static void Initialize_Blockchain();
        static Chain& Blockchain();
        Block& get_current_block();
        Block& get_block_at_index(unsigned int index);
        Block& create_new_block();
    };
}
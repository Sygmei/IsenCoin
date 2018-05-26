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
        std::vector<std::unique_ptr<Block>> m_blocks;
        static std::unique_ptr<Chain> s_blockchain;
        Chain();
        friend std::unique_ptr<Chain> std::make_unique<Chain>();
    public:
        bool auto_mining = true;
        int minimum_tx_amount = 1;
        static void Initialize_Blockchain();
        static Chain& Blockchain();
        Block& get_current_block();
        Block& get_block_at_index(unsigned int index);
        Block& create_new_block();
        void mine_and_next(bool force = false);
        void set_auto_mining(bool state);
    };
}
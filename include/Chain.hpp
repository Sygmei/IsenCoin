#pragma once

#include <memory>

#include <Block.hpp>
#include <Transaction.hpp>
#include <Wallet.hpp>

namespace ic
{
    class Tracker;

    class Chain 
    {
    private:
        std::vector<std::unique_ptr<Block>> m_blocks;
        Block null_block;
        static std::unique_ptr<Chain> s_blockchain;
        Chain();
        friend std::unique_ptr<Chain> std::make_unique<Chain>();
    public:
        bool auto_mining = false;
        int minimum_tx_amount = 1;
        static void Initialize_Blockchain();
        static Chain& Blockchain();
        Block& get_current_block();
        Block& get_block_at_index(unsigned int index);
        Block& create_new_block();
        void set_new_block(const Block& block);
        void mine_and_next(Tracker& tracker, bool force = false, public_key_t reward_recv = {});
        void set_auto_mining(bool state);
        std::vector<Block*> get_blocks() const;
        uint64_t get_blockchain_size() const;
        void save();
        void fetch_genesis(Tracker& tracker);
    };
}
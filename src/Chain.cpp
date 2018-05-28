#include <Chain.hpp>
#include <Exceptions.hpp>
#include "vili/ViliParser.hpp"
#include "Tracker.hpp"
#include "Utils.hpp"

namespace ic
{
    std::unique_ptr<Chain> Chain::s_blockchain;
    void Chain::Initialize_Blockchain()
    {
        s_blockchain = std::make_unique<Chain>();
    }

    Chain& Chain::Blockchain()
    {
        return *s_blockchain;
    }

    Chain::Chain()
    {
        vili::ViliParser blockchain_load;
        blockchain_load.parseFile("blockchain.vili");
        if (blockchain_load.root().getAll().empty())
        {
            /*if (!tracker.get_nodes().empty())
            {
                //tracker.propagate(p2p::build_msg("ask_block", { { "depth", 0 } }));
            }*/
        }
        else
        {
            for (const auto* blk : blockchain_load.root().getAll<vili::ComplexNode>())
            {
                Log->warn("Create new Block {}", blk->getId());
                m_blocks.push_back(std::make_unique<Block>(*blk));
            }
            //tracker.propagate(p2p::build_msg("ask_block", { { "depth", get_blockchain_size() } }));
        }
        if (m_blocks.empty())
        {
            Log->debug("Creating Blockchain... Mining genesis...");
            m_blocks.push_back(std::make_unique<Block>());
            get_current_block().mine(Block::mining_threads);
            get_current_block().validate();
        }
        create_new_block();
    }

    Block& Chain::get_current_block()
    {
        if (!m_blocks.empty())
            return *m_blocks.back();
        else
            return Block();
    }

    Block& Chain::get_block_at_index(unsigned int index)
    {
        return *m_blocks[index];
    }
    Block& Chain::create_new_block()
    {
        if (m_blocks.back()->is_valid())
        {
            save();
            m_blocks.push_back(std::make_unique<Block>(m_blocks.size(), m_blocks.back()->get_hash()));
            return get_current_block();
        }
        else
        {
            throw except::InvalidBlockException();
        }
    }

    void Chain::set_new_block(const Block& block)
    {
        if (
            (get_blockchain_size() > 1 && block.get_previous_hash() == get_block_at_index(get_blockchain_size() - 1).get_hash())
            || get_blockchain_size() <= 1)
        {
            m_blocks.pop_back();
            m_blocks.push_back(std::make_unique<Block>(block));
            get_current_block().validate();
            create_new_block();
        }
        else
        {
            Log->error("Could not push Block to Blockchain, incompatible previous_hash (Local : {} VS New : {})",
                char_array_to_hex(block.get_previous_hash()), char_array_to_hex(get_block_at_index(get_blockchain_size() - 1).get_hash()));
        }
    }

    void Chain::mine_and_next(Tracker& tracker, bool force, public_key_t reward_recv)
    {
        if (force || (auto_mining && get_current_block().get_tx_amount() >= minimum_tx_amount))
        {
            if (get_current_block().get_tx_amount() > 0)
            {
                get_current_block().interrupt();
                std::thread mining_thread([&]() {
                    get_current_block().mine(Block::mining_threads, reward_recv);
                    if (get_current_block().is_valid())
                    {
                        tracker.propagate(get_current_block().to_msgpack());
                        create_new_block();
                    }
                        
                });
                mining_thread.detach();
            }
            else
            {
                Log->warn("Can't start mining with 0 pending transactions, cancelling...");
            }
        }
    }

    std::vector<Block*> Chain::get_blocks() const
    {
        std::vector<Block*> blocks;
        for (const auto& blk : m_blocks)
        {
            blocks.push_back(blk.get());
        }
        return blocks;
    }

    uint64_t Chain::get_blockchain_size() const
    {
        return m_blocks.size();
    }

    void Chain::save()
    {
        vili::ViliParser blockchain_dump;
        for (const auto& blk : m_blocks)
            blk->dump(blockchain_dump.root());
        blockchain_dump.writeFile("blockchain.vili");
    }

    void Chain::fetch_genesis(Tracker& tracker)
    {
        if (m_blocks.size() <= 2)
        {
            Log->warn("Starting Blockchain force-sync...");
            m_blocks.clear();
            save();
            if (!tracker.get_nodes().empty())
            {
                tracker.propagate(p2p::build_msg("ask_block", { { "depth", 0 } }));
            }
            else
            {
                Log->debug("Creating Blockchain... Mining genesis...");
                m_blocks.push_back(std::make_unique<Block>());
                get_current_block().mine(Block::mining_threads);
                get_current_block().validate();
            }
        }
        else
        {
            Log->error("Can't force FetchGenesis with already started Blockchain");
        }
    }
}

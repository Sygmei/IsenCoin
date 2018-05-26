#include <Chain.hpp>
#include <Exceptions.hpp>

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
        Log->debug("Creating Blockchain... Mining genesis...");
        m_blocks.push_back(std::make_unique<Block>());
        get_current_block().mine(8);
        create_new_block();
    }

    Block& Chain::get_current_block()
    {
        return *m_blocks.back();
    }

    Block& Chain::get_block_at_index(unsigned int index)
    {
        return *m_blocks[index];
    }
    Block& Chain::create_new_block()
    {
        if (m_blocks.back()->is_valid())
        {
            m_blocks.push_back(std::make_unique<Block>(m_blocks.size(), m_blocks.back()->get_hash()));
            return get_current_block();
        }
        else
        {
            throw except::InvalidBlockException();
        }
    }

    void Chain::mine_and_next(bool force)
    {
        if (force || (auto_mining && get_current_block().get_tx_amount() >= minimum_tx_amount))
        {
            if (get_current_block().get_tx_amount() > 0)
            {
                std::thread mining_thread([&]() {
                    get_current_block().mine(8);
                    if (get_current_block().is_valid())
                        create_new_block();
                });
                mining_thread.detach();
            }
            else
            {
                Log->warn("Can't start mining with 0 pending transactions, cancelling...");
            }
        }
    }
}

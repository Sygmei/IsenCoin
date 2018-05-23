#include <Chain.hpp>
#include <Exceptions.hpp>

namespace ic
{
    Chain Blockchain;

    Chain::Chain()
    {
        initialize_logger();
        Log->debug("Creating Blockchain... Mining genesis...");
        m_current_block.mine(8);
    }

    Block& Chain::get_current_block()
    {
        return m_current_block;
    }

    Block& Chain::get_block_at_index(unsigned int index)
    {
        return *m_blocks[index].get();
    }
    Block& Chain::create_new_block()
    {
        if (m_current_block.is_valid())
        {
            m_blocks.emplace_back(std::make_unique<Block>(m_current_block));
            m_current_block = Block(*m_blocks.back().get());
            return m_current_block;
        }
        else
        {
            throw except::InvalidBlockException();
        }
    }
}
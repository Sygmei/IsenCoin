#include <Block.hpp>
#include <Config.hpp>
#include <picosha2.hpp>

#include <ed25519/sha512.h>
#include "base58/base58.hpp"
#include "Utils.hpp"
#include "Exceptions.hpp"

namespace ic 
{
    p2p::Requirements Block::Fields = {
        { "previous_hash", mp::MsgPack::Type::STRING },
        { "timestamp", mp::MsgPack::Type::UINT64 },
        { "nonce", mp::MsgPack::Type::UINT64 },
        { "depth", mp::MsgPack::Type::UINT64 }
    };

    int Block::mining_threads = config::DEFAULT_THREADS;
    bool Block::is_nonce_valid(block_hash_t& block_hash, uint64_t nonce) const
    {
        signature_t current_hash = Block::get_hash(block_hash, nonce);
        for (unsigned int i = 0; i < std::ceil(float(config::ISENCOIN_DIFFICULTY) / 2.0); i++)
        {
            const uint8_t& current_value = current_hash[i];
            if ((config::ISENCOIN_DIFFICULTY % 2) && i == (config::ISENCOIN_DIFFICULTY / 2))
                return (current_value >> 4) == 0;
            if (current_value)
                return false;
        }
        return true;
    }

    void Block::generate_timestamp()
    {
        std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
        m_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
    }

    signature_t Block::calculate_merkle_root()
    {
        if (!m_transactions.empty())
        {
            std::vector<signature_t> transactions_signatures;
            for (const auto& tx : m_transactions)
                transactions_signatures.push_back(tx->get_signature());
            return Transaction::get_merkel_root(transactions_signatures);
        }
        else
            return {};
    }

    void Block::dump(vili::ComplexNode& node)
    {
        if (is_valid())
        {
            vili::ComplexNode& blk = node.createComplexNode(get_hex_hash());
            blk.createDataNode("timestamp", m_timestamp);
            blk.createDataNode("depth", m_depth);
            blk.createDataNode("nonce", m_nonce);
            blk.createDataNode("previous_hash", base58::encode(m_previous_hash.data(), m_previous_hash.data() + m_previous_hash.size()));
            if (m_transactions.size() > 0)
            {
                vili::ComplexNode& txs = blk.createComplexNode("txs");
                for (const auto& tx : m_transactions)
                    tx->dump(txs);
                validate();
            }
        }
    }

    mp::MsgPack Block::to_msgpack(bool asked) const
    {
        mp::MsgPack::array txs;
        for (const auto& tx : m_transactions)
            txs.push_back(tx->to_msgpack());
        return p2p::build_msg("block", {
            { "timestamp", m_timestamp },
            { "previous_hash", base58::encode(m_previous_hash.data(), m_previous_hash.data() + m_previous_hash.size()) },
            { "depth", m_depth },
            { "nonce", m_nonce },
            { "txs", txs },
            { "asked", asked }
        });
    }

    signature_t Block::get_previous_hash() const
    {
        return m_previous_hash;
    }

    block_hash_t Block::fill_block_hash()
    {
        block_hash_t block_hash = {};
        uint64_t* depth_slot = reinterpret_cast<uint64_t*>(block_hash.data());
        *depth_slot = m_depth;
        std::copy(m_previous_hash.begin(), m_previous_hash.end(), block_hash.begin() + sizeof(uint64_t));
        signature_t temp_merkle_root = calculate_merkle_root();
        std::copy(temp_merkle_root.begin(), temp_merkle_root.end(), block_hash.data() + sizeof(uint64_t) + sizeof(signature_t));
        //Log->warn("BLOCK HASH : {}", char_array_to_hex(block_hash));
        return block_hash;
    }

    Block::Block()
    {
        m_previous_hash = {};
        m_current_hash = {};
        m_depth = 0;
        generate_timestamp();
        fill_block_hash();
    }

	Block::Block(const Block& block)
	{
		m_current_hash = block.m_current_hash;
		m_nonce = block.m_nonce;
		m_previous_hash = block.m_previous_hash;
		m_timestamp = block.m_timestamp;
        m_depth = block.m_depth;
        validate();
	}

    Block::Block(uint64_t index, signature_t previous_hash, std::vector<Transaction> transactions)
    {
        m_previous_hash = previous_hash;
        for (const auto& tx : transactions)
            m_transactions.push_back(std::make_unique<Transaction>(tx));
        m_depth = index;
        generate_timestamp();
        m_current_hash = fill_block_hash();
    }

    Block::Block(uint64_t index, signature_t previous_hash, uint64_t nonce, timestamp_t timestamp, std::vector<Transaction> transactions)
    {
        m_previous_hash = previous_hash;
        for (const auto& tx : transactions)
        {
            Log->debug("Building Block containing transaction {}", tx.as_string());
            m_transactions.push_back(std::make_unique<Transaction>(tx));
        }
            
        m_nonce = nonce;
        m_timestamp = timestamp;
        m_depth = index;
        m_current_hash = fill_block_hash();
        validate();
    }

    Block::Block(const vili::ComplexNode& node)
    {
        m_timestamp = node.at<vili::DataNode>("timestamp").get<unsigned long long>();
        std::string hash_vili = node.at<vili::DataNode>("previous_hash").get<std::string>();
        std::vector<unsigned char> hash_dcode;
        base58::decode(hash_vili.c_str(), hash_dcode);
        std::copy(hash_dcode.begin(), hash_dcode.end(), m_previous_hash.begin());
        m_depth = node.at<vili::DataNode>("depth").get<unsigned long long>();
        m_nonce = node.at<vili::DataNode>("nonce").get<unsigned long long>();
        if (node.contains("txs"))
        {
            for (const auto* tx : node.at("txs").getAll<vili::ComplexNode>())
                m_transactions.push_back(std::make_unique<Transaction>(*tx));
        }
        
        m_current_hash = fill_block_hash();
        validate();
    }

    Block& Block::operator=(const Block& block)
    {
        m_current_hash = block.m_current_hash;
        m_nonce = block.m_nonce;
        m_previous_hash = block.m_previous_hash;
        m_timestamp = block.m_timestamp;
        m_depth = block.m_depth;
        for (const auto& tx : block.m_transactions)
            m_transactions.push_back(std::make_unique<Transaction>(*tx));
        return *this;
    }

    void Block::add_transaction(const Transaction& tx)
    {
        Transaction ltx = tx;
        if (ltx.validate())
        {
            m_mining = false;
            Log->warn("Disabling mining for Block with timestamp (tx_add) {}", m_timestamp);
            m_transactions.push_back(std::make_unique<Transaction>(tx));
            m_validated = false;
            generate_timestamp();
            Log->info("Added new transaction : {}", ltx.as_string());
        }
        else
        {
            Log->error("Can't add invalid Transaction to Block : {}", ltx.as_string());
        }
    }

    void Block::validate()
    {
        Log->warn("Validating N{} T{} D{} P{} C{}", m_nonce, m_timestamp, m_depth, base58::encode(m_previous_hash.data(), m_previous_hash.data() + m_previous_hash.size()), base58::encode(m_current_hash.data(), m_current_hash.data() + m_current_hash.size()));
        if (is_nonce_valid(m_current_hash, m_nonce))
            m_validated = true;
        else
            throw except::InvalidBlockException();
    }

    signature_t Block::get_hash(block_hash_t& block_hash, uint64_t nonce) const
    {
        timestamp_t* timestamp_slot = reinterpret_cast<timestamp_t*>(block_hash.data() + 2 * sizeof(signature_t) + sizeof(uint64_t));
        *timestamp_slot = m_timestamp;
        uint64_t* nonce_slot = reinterpret_cast<uint64_t*>(block_hash.data() + 2 * sizeof(signature_t) + sizeof(timestamp_t) + sizeof(uint64_t));
        *nonce_slot = nonce;
        signature_t final_hash;
        sha512(block_hash.data(), block_hash.size(), final_hash.data());
        return final_hash;
    }

    signature_t Block::get_hash()
    {
        if (m_validated)
            return Block::get_hash(m_current_hash, m_nonce);
        else
            throw except::InvalidBlockException();
    }

    std::string Block::get_hex_hash()
    {
        return char_array_to_hex(get_hash());
    }

    uint64_t Block::get_nonce() const
    {
        return m_nonce;
    }

    void Block::mine(uint8_t threads, public_key_t reward_recv)
    {
		while (m_mining || m_sagain) {}
        Log->warn("Enabling mining for Block with timestamp {}", m_timestamp);
        if (reward_recv != config::ISENCOIN_NULL_ADDRESS)
        {
            const Transaction reward_tx(reward_recv);
            add_transaction(reward_tx);
        }
        m_mining = true;
        m_sagain = true;
        std::vector<std::thread> thread_pool;
        Log->debug("Starting mining");
        block_hash_t thread_base_hash = fill_block_hash();
        for (uint8_t thread_index = 0; thread_index < threads; thread_index++)
        {
            thread_pool.emplace_back([&](block_hash_t thread_hash, uint8_t starting_index, uint8_t increment_level)
            {
                unsigned long long nonce = starting_index;
  
                while (m_mining && !m_validated && !is_nonce_valid(thread_hash, nonce))
                {
                    nonce += increment_level;
                    if ((nonce % 1000000) == 0)
                        Log->debug("Nonce : {} = {}", nonce, char_array_to_hex(thread_hash));
                }
                if (m_mining)
                {
                    if (!m_validated)
                    {
                        m_nonce = nonce;
                        m_current_hash = thread_hash;
						Log->trace("Validating BLOCK !");
						m_validated = true;
                    }
                }
                else
                {
                    Log->warn("Current Block mining has been interrupted !");
                }
            }, thread_base_hash, thread_index, threads);
        }
        for (std::thread &t : thread_pool) {
            if (t.joinable()) {
                t.join();
            }
        }

		if (m_mining && m_validated)
		{
			Log->debug("Mining ended with nonce {}", m_nonce);
            Log->debug("Block parameters : [\n nonce:{},\n timestamp:{}\n depth:{},\n previous_hash:{}\n merkle_root:{}\n];", 
                m_nonce, 
                m_timestamp,
                m_depth, 
                char_array_to_hex(m_previous_hash),
                char_array_to_hex(calculate_merkle_root()));
			const signature_t f_hash = get_hash();
			Log->debug("Resulting in following hash : {}", char_array_to_hex(f_hash));
		}
        else
        {
            Log->warn("Mining aborted, removing reward from Block");
            clean_block();
        }
        Log->warn("Disabling mining for Block with timestamp {}", m_timestamp);
		m_mining = false;
        m_sagain = false;
        Log->debug("Is Block Validated ? {}", m_validated);
    }

    bool Block::is_valid() const
    {
        return m_validated;
    }

    bool Block::is_mining() const
    {
        return m_mining;
    }

    unsigned Block::get_tx_amount() const
    {
        return m_transactions.size();
    }

    std::vector<Transaction*> Block::get_transactions() const
    {
        std::vector<Transaction*> txs;
        for (const auto& tx : m_transactions)
            txs.push_back(tx.get());
        return txs;
    }

    unsigned int Block::get_depth() const
    {
        return m_depth;
    }

    void Block::clean_block()
    {
        m_transactions.erase(std::remove_if(m_transactions.begin(), m_transactions.end(), [](const auto& tx)
        {
            return (tx->is_reward());
        }), m_transactions.end());
    }

    void Block::interrupt()
    {
        m_mining = false;
    }

    timestamp_t Block::get_timestamp() const
    {
        return m_timestamp;
    }
}


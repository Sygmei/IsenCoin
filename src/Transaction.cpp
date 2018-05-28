#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

#include <ed25519/ed25519.h>
#include <ed25519/sha512.h>
#include <msgpack11/msgpack11.hpp>

#include <Config.hpp>
#include <Transaction.hpp>
#include <Utils.hpp>
#include "base58/base58.hpp"
#include "Exceptions.hpp"

namespace ic
{
    namespace mp = msgpack11;
    p2p::Requirements Transaction::Fields = {
        { "signature", mp::MsgPack::Type::STRING },
        { "timestamp", mp::MsgPack::Type::UINT64 },
        { "amount", mp::MsgPack::Type::FLOAT32 },
        { "sender", mp::MsgPack::Type::STRING },
        { "receiver", mp::MsgPack::Type::STRING }
    };

    Transaction::Transaction(const Transaction& tx)
    {
        m_timestamp = tx.m_timestamp;
        m_amount = tx.m_amount;
        m_receiver = tx.m_receiver;
        m_sender = tx.m_sender;
        m_signature = tx.m_signature;
    }

    Transaction::Transaction(const Wallet& sender, const Wallet& receiver, float amount)
    {
        m_sender = sender.get_public_key();
        m_receiver = receiver.get_public_key();
        m_amount = amount;
        std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
        m_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();

        std::string tx_sign_message = this->get_signable_transaction_message();
        ed25519_sign(
            m_signature.data(), 
            reinterpret_cast<const unsigned char *>(tx_sign_message.c_str()),
            tx_sign_message.size(),
            sender.get_public_key().data(), 
            sender.get_private_key().data()
        );
    }

    Transaction::Transaction(public_key_t receiver)
    {
        m_sender = config::ISENCOIN_NULL_ADDRESS;
        m_receiver = receiver;
        m_amount = config::ISENCOIN_REWARD;
        std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
        m_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();

        std::string tx_sign_message = this->get_signable_transaction_message();
        ed25519_sign(
            m_signature.data(),
            reinterpret_cast<const unsigned char *>(tx_sign_message.c_str()),
            tx_sign_message.size(),
            config::ISENCOIN_NULL_ADDRESS.data(),
            config::ISENCOIN_NULL_PV_ADDRESS.data()
        );
    }

    Transaction::Transaction(const Wallet& sender, const public_key_t& receiver, float amount)
    {
        m_sender = sender.get_public_key();
        m_receiver = receiver;
        m_amount = amount;
        std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
        m_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();

        std::string tx_sign_message = this->get_signable_transaction_message();
        ed25519_sign(
            m_signature.data(),
            reinterpret_cast<const unsigned char *>(tx_sign_message.c_str()),
            tx_sign_message.size(),
            sender.get_public_key().data(),
            sender.get_private_key().data()
        );
    }

    Transaction::Transaction(const public_key_t& sender, const public_key_t& receiver, const amount_t amount, const timestamp_t timestamp,
                             const signature_t& signature)
    {
        m_sender = sender;
        m_receiver = receiver;
        m_amount = amount;
        m_timestamp = timestamp;
        m_signature = signature;
    }

    bool Transaction::validate()
    {
        std::string tx_sign_message = this->get_signable_transaction_message();
        if (ed25519_verify(m_signature.data(), reinterpret_cast<const unsigned char*>(tx_sign_message.c_str()), tx_sign_message.size(), m_sender.data()))
        {
            Log->info("Valid Transaction ! {}", this->as_string());
            if (m_sender == config::ISENCOIN_NULL_ADDRESS && m_amount != config::ISENCOIN_REWARD)
                throw except::InvalidRewardAmountException(m_amount);
            if (m_amount < 0)
                throw except::NegativeAmountException(m_amount);
            return true;
        }
        else
        {
            Log->warn("Invalid Transaction : {}", this->as_string());
            //throw except::InvalidTransactionException();
            return false;
        }
            
        /*if (m_sender != 0)
        {

        }*/
        /*else if (m_amount != config::)
        {
            throw std::runtime_error("Invalid Reward Amount");
        }*/
    }

    /*float Transaction::balance(int address)
    {
        if (address == m_sender)
            return -m_amount;
        else if (address == m_receiver)
            return m_amount;
        return 0;
    }*/

    signature_t Transaction::combine(const signature_t& signature1, const signature_t& signature2)
    {
        std::array<unsigned char, 128> input;
        signature_t result;
        std::copy(signature1.begin(), signature1.end(), input.begin());
        std::copy(signature2.begin(), signature2.end(), input.begin() + signature1.size());
        sha512(input.data(), input.size(), result.data());
        return result;
    }

    Transaction::Transaction(const mp::MsgPack& msg)
    {
        Log->warn("Building Tx from MsgPack : {}", msg.dump());
        p2p::decode_b58(msg["sender"].string_value(), m_sender);
        p2p::decode_b58(msg["receiver"].string_value(), m_receiver);
        p2p::decode_b58(msg["signature"].string_value(), m_signature);

        m_amount = msg["amount"].float32_value();
        m_timestamp = msg["timestamp"].uint64_value();
    }

    Transaction::Transaction(const vili::ComplexNode& node)
    {
        m_timestamp = node.at<vili::DataNode>("timestamp").get<uint64_t>();
        m_amount = node.at<vili::DataNode>("amount").get<double>();
        std::string sender_vili = node.at<vili::DataNode>("sender").get<std::string>();
        std::vector<unsigned char> sender_dcode;
        base58::decode(sender_vili.c_str(), sender_dcode);
        std::copy(sender_dcode.begin(), sender_dcode.end(), m_sender.begin());
        std::string receiver_vili = node.at<vili::DataNode>("receiver").get<std::string>();
        std::vector<unsigned char> receiver_dcode;
        base58::decode(receiver_vili.c_str(), receiver_dcode);
        std::copy(receiver_dcode.begin(), receiver_dcode.end(), m_receiver.begin());
        std::string signature_vili = node.at<vili::DataNode>("signature").get<std::string>();
        std::vector<unsigned char> signature_dcode;
        base58::decode(signature_vili.c_str(), signature_dcode);
        std::copy(signature_dcode.begin(), signature_dcode.end(), m_signature.begin());
    }

    const signature_t& Transaction::get_signature() const
    {
        return m_signature;
    }

    std::string Transaction::get_printable_signature() const
    {
        return char_array_to_hex(m_signature);
    }

    timestamp_t Transaction::get_timestamp() const
    {
        return m_timestamp;
    }

    float Transaction::get_amount() const
    {
        return m_amount;
    }

    const public_key_t& Transaction::get_sender() const
    {
        return m_sender;
    }

    const public_key_t& Transaction::get_receiver() const
    {
        return m_receiver;
    }

    mp::MsgPack Transaction::to_msgpack() const
    {
        return p2p::build_msg("transaction", {
            { "amount", m_amount },
            { "timestamp", m_timestamp },
            { "signature", base58::encode(m_signature.data(), m_signature.data() + m_signature.size()) },
            { "sender", base58::encode(m_sender.data(), m_sender.data() + m_sender.size()) },
            { "receiver", base58::encode(m_receiver.data(), m_receiver.data() + m_receiver.size()) },
        });
    }

    void Transaction::corrupt()
    {
        m_amount += 1;
        //m_timestamp += 1;
        //m_sender[31] = m_sender[11];
        //m_receiver[0] = 0;
        //m_signature[0] = 10;
    }

    std::string Transaction::as_string() const
    {
        std::stringstream ss;
        ss << "Transaction data : {" << std::endl;
        ss << "    sender : " << base58::encode(m_sender.data(), m_sender.data() + m_sender.size()) << "," << std::endl;
        ss << "    receiver : " << base58::encode(m_receiver.data(), m_receiver.data() + m_receiver.size()) << "," << std::endl;
        ss << "    amount : " << m_amount << "," << std::endl;
        ss << "    timestamp : " << m_timestamp << "," << std::endl;
        ss << "    signature : " << base58::encode(m_signature.data(), m_signature.data() + m_signature.size()) << std::endl;
        ss << "};";
        return ss.str();
    }

    std::string Transaction::get_signable_transaction_message()
    {
        namespace txfc = config::tx_field_chars;
        std::stringstream ss;
        ss << txfc::timestamp << m_timestamp;
        ss << txfc::amount << m_amount;
        ss << txfc::sender << std::string(m_sender.begin(), m_sender.end());
        ss << txfc::receiver << std::string(m_receiver.begin(), m_receiver.end());
        return ss.str();
    }

    std::string Transaction::get_b58_signature() const
    {
        return base58::encode(m_signature.data(), m_signature.data() + m_signature.size());
    }

    bool Transaction::is_reward() const
    {
        return (m_sender == config::ISENCOIN_NULL_ADDRESS);
    }

    void Transaction::dump(vili::ComplexNode& node)
    {
        vili::ComplexNode& tx = node.createComplexNode(this->get_printable_signature());
        tx.createDataNode("timestamp", m_timestamp);
        tx.createDataNode("amount", m_amount);
        tx.createDataNode("sender", base58::encode(m_sender.data(), m_sender.data() + m_sender.size()));
        tx.createDataNode("receiver", base58::encode(m_receiver.data(), m_receiver.data() + m_receiver.size()));
        tx.createDataNode("signature", get_b58_signature());
        
    }

    signature_t Transaction::get_merkel_root(const std::vector<signature_t>& signatures)
    {
        std::vector<signature_t> signatures_out = signatures;

        std::sort(signatures_out.begin(), signatures_out.end(), [](const signature_t& a, const signature_t& b)
        {
            return (a < b);
        });
        if (signatures_out.empty())
        {
            throw std::runtime_error("Can't get merkle root of empty signatures vector");
        }
        while (signatures_out.size() > 1)
        {
            std::vector<signature_t> signatures_buffer;
            signature_t even_signature;
            if (signatures_out.size() % 2 != 0)
            {
                Log->trace("Even amount of signatures, copying last one...");
                signatures_out.push_back(signatures_out.back());
            }
            for (unsigned int i = 0; i < signatures_out.size(); i += 2)
            {
                signature_t& combine_result = signatures_buffer.emplace_back(Transaction::combine(signatures_out[i], signatures_out[i + 1]));
                Log->trace("Combine : {} and {} = {}",
                    base58::encode(signatures_out[i].data(), signatures_out[i].data() + signatures_out[i].size()),
                    base58::encode(signatures_out[i + 1].data(), signatures_out[i + 1].data() + signatures_out[i + 1].size()),
                    base58::encode(combine_result.data(), combine_result.data() + combine_result.size()));
            }
            signatures_out = signatures_buffer;
            signatures_buffer.clear();
            Log->trace("<== BRANCH END ==>");
        }

        return signatures_out[0];
    }
}

#pragma once

#include <cstdint>
#include <string>

#include <P2P.hpp>
#include <Types.hpp>
#include <Wallet.hpp>

namespace ic
{
    namespace mp = msgpack11;
    class Transaction 
    {
    private:
        signature_t m_signature;
        timestamp_t m_timestamp;
        amount_t m_amount;
        public_key_t m_sender;
        public_key_t m_receiver;
        std::string get_signable_transaction_message();
    public:
        static signature_t get_merkel_root(const std::vector<signature_t>& signatures);
        static signature_t combine(const signature_t& signature1, const signature_t& signature2);
        Transaction(const Wallet& sender, const Wallet& receiver, float amount);
        Transaction(const public_key_t& sender, const public_key_t& receiver, amount_t amount, timestamp_t timestamp, const signature_t& signature);
        void validate();
        const signature_t& get_signature() const;
        std::string get_printable_signature() const;
        timestamp_t get_timestamp() const;
        amount_t get_amount() const;
        const public_key_t& get_sender() const;
        const public_key_t& get_receiver() const;
        mp::MsgPack to_msgpack() const;
        void corrupt();
        std::string as_string() const;
        //float balance(int address);
        static p2p::Requirements Fields;
    };
}

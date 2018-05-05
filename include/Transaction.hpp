#pragma once

#include <cstdint>
#include <string>

#include <Wallet.hpp>

namespace ic
{
    using signature_t = std::array<unsigned char, 64>;
    class Transaction 
    {
    private:
        signature_t m_signature;
        uint32_t m_timestamp;
        float m_amount;
        public_key_t m_sender;
        public_key_t m_receiver;
    public:
        static signature_t get_merkel_root(const std::vector<signature_t>& signatures);
        static signature_t combine(const signature_t& signature1, const signature_t& signature2);
        Transaction(const Wallet& sender, const Wallet& receiver, float amount);
        void validate();
        const signature_t& get_signature() const;
        std::string get_printable_signature() const;
        //float balance(int address);
    };
}
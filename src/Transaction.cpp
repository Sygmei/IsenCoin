#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

#include <ed25519/ed25519.h>
#include <ed25519/sha512.h>

#include <Config.hpp>
#include <Transaction.hpp>
#include <Utils.hpp>

namespace ic
{
    Transaction::Transaction(const Wallet& sender, const Wallet& receiver, float amount)
    {
        m_sender = sender.get_public_key();
        m_receiver = receiver.get_public_key();
        m_amount = amount;
        std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
        m_timestamp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();

        std::string message = std::to_string(m_timestamp) + "_" + std::to_string(m_amount) + "_";
        for (const auto& e : sender.get_public_key())
            message += e;
        message += "_";
        for (const auto& e : receiver.get_public_key())
            message += e;
        std::cout << "Transaction data : {" << std::endl;
        std::cout << "    sender : " << char_array_to_hex(m_sender) << "," << std::endl;
        std::cout << "    receiver : " << char_array_to_hex(m_receiver) << "," << std::endl;
        std::cout << "    amount : " << m_amount << "," << std::endl;
        std::cout << "    timestamp : " << m_timestamp << "," << std::endl;
        std::cout << "}" << std::endl;
        std::cout << "Transaction Data HexFormat : " << char_array_to_hex(message) << std::endl;
        const unsigned char* message_cstr = reinterpret_cast<const unsigned char *>(message.c_str());
        ed25519_sign(m_signature.data(), message_cstr, message.size(), sender.get_public_key().data(), sender.get_private_key().data());
        std::string str_sign(std::begin(m_signature), std::end(m_signature));
        //std::string encoded = Base58::base58().encode(str_sign);
        //std::cout << "Signature : " <<this->get_printable_signature() << std::endl;
    }

    void Transaction::validate()
    {
        /*if (m_sender != 0)
        {

        }
        else if (m_amount != config::reward)
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
    
    const signature_t& Transaction::get_signature() const
    {
        return m_signature;
    }

    std::string Transaction::get_printable_signature() const
    {
        return char_array_to_hex(m_signature);
    }

    signature_t Transaction::get_merkel_root(const std::vector<signature_t>& signatures)
    {
        std::vector<signature_t> signatures_out = signatures;
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
                std::cout << "Even amount of signatures, excluding last one..." << std::endl;
                signatures_buffer.push_back(signatures_out.front());
                signatures_out.erase(signatures_out.begin());
            }
            for (unsigned int i = 0; i < signatures_out.size(); i+= 2)
            {
                signatures_buffer.push_back(Transaction::combine(signatures_out[i], signatures_out[i + 1]));
                std::cout << "Combine : " << char_array_to_hex(signatures_out[i]) << " and " << char_array_to_hex(signatures_out[i + 1]) << std::endl;
                std::cout << "  => Got signature : " << char_array_to_hex(Transaction::combine(signatures_out[i], signatures_out[i + 1])) << std::endl;
            }
            signatures_out = signatures_buffer;
            signatures_buffer.clear();
            std::cout << "<== BRANCH END ==>" << std::endl;
        }

        return signatures_out[0];
    }
}
#include <stdexcept>

#include <Config.hpp>
#include <Transaction.hpp>

namespace ic
{
    Transaction::Transaction(int sender, int receiver, float amount)
    {
        m_sender = sender;
        m_receiver = receiver;
        m_amount = amount;
    }

    int Transaction::sign(int key)
    {
        return 0;
    }

    void Transaction::validate()
    {
        if (m_sender != 0)
        {

        }
        else if (m_amount != config::reward)
        {
            throw std::runtime_error("Invalid Reward Amount");
        }
    }

    float Transaction::balance(int address)
    {
        if (address == m_sender)
            return -m_amount;
        else if (address == m_receiver)
            return m_amount;
        return 0;
    }
}
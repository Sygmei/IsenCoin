#pragma once

namespace ic
{
    class Transaction 
    {
    private:
        int m_signature;
        int m_time;
        float m_amount;
        int m_sender;
        int m_receiver;
    public:
        Transaction(int sender, int receiver, float amount);
        int sign(int key);
        void validate();
        float balance(int address);
    };
}
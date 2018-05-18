#pragma once

#include <vector>

#include <Transaction.hpp>

namespace ic
{
    class Block 
    {
    private:
        std::vector<Transaction> m_transactions;
        int depth = 0;
        int nonce = 0;
        int m_current_hash = 0;
        int m_previous_hash = 0;
        int m_time = 0;    
    public:
        void validate();
		const int& get_current_hash() const;
		const int& get_previous_hash() const;
		const int& get_time() const;
		const std::vector<Transaction>& get_transactions() const;
		void set_transaction(std::vector<Transaction> transactions);
    };
}
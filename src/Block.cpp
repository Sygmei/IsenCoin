#include <Block.hpp>

#include <ed25519/sha512.h>

namespace ic 
{
    void Block::validate()
    {
    }

	// Getter
	const int & Block::get_current_hash() const
	{
		return m_current_hash;
	}
	const int & Block::get_previous_hash() const
	{
		return m_previous_hash;
	}
	const int & Block::get_time() const
	{
		return m_time;
	}
	const std::vector<Transaction>& Block::get_transactions() const
	{
		return m_transactions;
	}

	// Setter
	void Block::set_transaction(std::vector<Transaction> transactions)
	{
		m_transactions = transactions;
	}
}
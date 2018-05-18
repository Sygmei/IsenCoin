#pragma once

#include <sstream>
#include <fstream>

#include <Serialization.pb.hpp>
#include <Transaction.hpp>
#include <Block.hpp>
#include <Wallet.hpp>
#include <Utils.hpp>
#include <Functions.hpp>

namespace ic {
	// Transactions
	void DataStockTransaction(Transaction transaction, TransactionSer* transdata);
	void EncodeTransaction();

	// Blocks
	void DataStockBlock(Block block, BlockSer* blockdata);

	// File
	void WriteData(long long nbFile, FileSer stockdata, BlockSer blockdata, Block block);
}
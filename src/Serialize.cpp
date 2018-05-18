#include <Serialize.hpp>

#define MAX_FILE_SIZE 1
#define FILENAME "ICBlockChain"
#define FILE_EXTENSION ".bin"

// Transactions
void ic::DataStockTransaction(Transaction transaction, TransactionSer* transdata)
{
	std::string str_receiver(transaction.get_receiver().begin(), transaction.get_receiver().end());
	std::string str_sender(transaction.get_sender().begin(), transaction.get_sender().end());
	std::string str_signature(transaction.get_signature().begin(), transaction.get_signature().end());


	transdata->set_receiver(str_receiver);
	transdata->set_amout(transaction.get_amount());
	transdata->set_sender(str_sender);
	transdata->set_signature(str_signature);
	transdata->set_timestamp(transaction.get_timestamp());
}

void ic::EncodeTransaction()
{
}

// Blocks
void ic::DataStockBlock(Block block,BlockSer* blockdata)
{
	blockdata->set_current_hash(block.get_current_hash());
	blockdata->set_previous_hash(block.get_previous_hash());
	blockdata->set_time(block.get_time());
}

// Writing the block
void ic::WriteData(long long nbFile, FileSer stockdata, BlockSer blockdata, Block block)
{
	std::string filename;
	std::stringstream concatenationFilename;
	concatenationFilename << FILENAME << nbFile << FILE_EXTENSION;
	filename = concatenationFilename.str();

	std::fstream input(filename, std::ios::in | std::ios::binary);
	if (!input)
	{
		std::cout << filename << ": File not found.  Creating a new file." << std::endl;
	}
	else if (ic::utils::getFileSize(filename) >= 80)
	{
		std::cout << filename << ": File size too heavy..." << std::endl;
		input.close();
		nbFile += 1;
		concatenationFilename << FILENAME << nbFile << FILE_EXTENSION;
		filename = concatenationFilename.str();
	}
	else if (!stockdata.ParseFromIstream(&input)) 
	{
		std::cout << "Failed to parse the file." << std::endl;
		exit(EXIT_FAILURE);
		input.close();
	}
	input.close();

	for (auto transaction : block.get_transactions())
	{
		DataStockTransaction(transaction, blockdata.add_transaction());
	}
	DataStockBlock(block, stockdata.add_block());

	std::fstream output(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!stockdata.SerializeToOstream(&output)) {
		std::cout << "Failed to write address book." << std::endl;
	}
	

}

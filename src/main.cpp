#include <cmath>
#include <iostream>

#include <msgpack11/msgpack11.hpp>
#include <tacopie/tacopie>
#include <vili/Vili.hpp>

#include <argh.h>
#include <Chain.hpp>
#include <Config.hpp>
#include <Logger.hpp>
#include <Message.hpp>
#include <P2P.hpp>
#include <Tracker.hpp>
#include <Transaction.hpp>
#include <Utils.hpp>

#include <ed25519/fixedint.h>

std::condition_variable cv;

void
signint_handler(int) {
  cv.notify_all();
}

int main(int argc, char** argv)
{
    tacopie::init();

    argh::parser cmdl;
    cmdl.add_param("prefix");
    cmdl.add_param("prefix2");
    cmdl.add_param("port");
    cmdl.parse(argc, argv);

    using namespace ic;
    using namespace msgpack11;

    initialize_logger();

    MsgPack my_msgpack = MsgPack::object {
        { "key1", "value1" },
        { "key2", false },
        { "key3", MsgPack::array { 1, 2, 3 } },
    };
    
    //access to elements
    std::cout << my_msgpack["key1"].string_value() << std::endl;
    
    //serialize
    std::string msgpack_bytes = my_msgpack.dump();
    std::cout << "Bytes : " << msgpack_bytes << std::endl;
    
    //deserialize
    std::string err;
    MsgPack des_msgpack = MsgPack::parse(msgpack_bytes, err);
    
    unsigned int port;
    if (cmdl("port"))
    {
        try 
        {
            port = std::stoi(cmdl("port").str());
        }
        catch (const std::invalid_argument& e)
        {
            Log->warn("Invalid port : \"{}\", using default port 15317", cmdl("port").str());
            port = ic::config::DEFAULT_PORT;
        }
    }
    else
    {
        port = ic::config::DEFAULT_PORT;
    }
    Tracker tracker(port);
    /*vili::ViliParser config_file;
    config_file.parseFile("config.vili");
    for (const vili::DataNode* ip : config_file.at<vili::ArrayNode>("ips"))
        tracker.add_node(Node(ip->get<std::string>()));*/

    const unsigned int prefix_size = cmdl("prefix").str().size();

    std::cout << sizeof(uint32_t) << " or " << sizeof(int) << std::endl;

    Chain chain;
    //chain.create_wallet();

    Wallet myWallet(false);
    Wallet myWallet2(false);

    std::cout << "Starting IsenCoin Program..." << std::endl;
    std::cout << "Benchmarking Wallet creation..." << std::endl;
    const unsigned long long int input_seconds = (double(std::pow(58, prefix_size)) * Wallet::Benchmark()) / 1000000.0;
    const unsigned int days = input_seconds / 60 / 60 / 24;
    const unsigned int hours = (input_seconds / 60 / 60) % 24;
    const unsigned int minutes = (input_seconds / 60) % 60;
    const unsigned int seconds = input_seconds % 60;
    std::cout << "Asked prefix will take approximatively " << days << "d " << hours << "h " << minutes << "m " << seconds << "s" << std::endl;
    myWallet.generate(cmdl("prefix").str());
    myWallet2.generate(cmdl("prefix2").str());
    auto pub = myWallet.get_public_key();
    std::string str_pub(std::begin(pub), std::end(pub));

    std::vector<Transaction> txs;
    for (unsigned int i = 1; i < 4; i++)
    {
        txs.emplace_back(myWallet, myWallet2, i);
    }
    std::vector<signature_t> sgns;
    for (Transaction& tx : txs)
    {
        sgns.emplace_back(tx.get_signature());
    }

    const std::string merkle_root = char_array_to_hex(Transaction::get_merkel_root(sgns));
    std::cout << "Merkel Root is : " << merkle_root << std::endl;

    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);

    tacopie::close();

    return 0;
}
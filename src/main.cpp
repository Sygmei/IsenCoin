#include <cmath>
#include <iostream>

#include <base_x/base_x.hh>
#include <msgpack11/msgpack11.hpp>
#include <tacopie/tacopie>

#include <argh.h>
#include <Chain.hpp>
#include <Transaction.hpp>
#include <Utils.hpp>

int main(int argc, char** argv)
{
    using namespace msgpack11;

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

    tacopie::tcp_server s;
    s.start("127.0.0.1", 3001, [] (const std::shared_ptr<tacopie::tcp_client>& client) -> bool {
      std::cout << "New client" << std::endl;
      client->async_read({1024, [client](const tacopie::tcp_client::read_result& res) {
          if (res.success) {
              std::string response = "HTTP/1.1 200 OK\nContent-Length: 1\nContent-Type: text/plain\na";
              std::vector<char> res_vec(response.begin(), response.end());
              std::cout << "Success" << std::endl;
              client->async_write({res_vec, nullptr});
          }
          else {
              std::cout << "Failed :(" << std::endl;
          }
      }});
      return true;
    });

    using namespace ic;
    argh::parser cmdl;
    cmdl.add_param("prefix");
    cmdl.add_param("prefix2");
    cmdl.parse(argc, argv);
    unsigned int prefix_size = cmdl("prefix").str().size();

    std::cout << sizeof(uint32_t) << " or " << sizeof(int) << std::endl;

    Chain chain;
    //chain.create_wallet();

    Wallet myWallet(false);
    Wallet myWallet2(false);

    std::cout << "Starting IsenCoin Program..." << std::endl;
    std::cout << "Benchmarking Wallet creation..." << std::endl;
    unsigned long long int input_seconds = (double(std::pow(58, prefix_size)) * Wallet::Benchmark()) / 1000000.0;
    unsigned int days = input_seconds / 60 / 60 / 24;
    unsigned int hours = (input_seconds / 60 / 60) % 24;
    unsigned int minutes = (input_seconds / 60) % 60;
    unsigned int seconds = input_seconds % 60;
    std::cout << "Asked prefix will take approximatively " << days << "d " << hours << "h " << minutes << "m " << seconds << "s" << std::endl;
    myWallet.generate(cmdl("prefix").str());
    myWallet2.generate(cmdl("prefix2").str());
    auto pub = myWallet.get_public_key();
    std::string str_pub(std::begin(pub), std::end(pub));
    auto encoded = Base58::base58().encode(str_pub);
    std::cout << "isen_" + encoded << std::endl;

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
    
    std::string merkle_root = char_array_to_hex(Transaction::get_merkel_root(sgns));
    std::cout << "Merkel Root is : " << merkle_root << std::endl;

    while (true) {}

    return 0;
}
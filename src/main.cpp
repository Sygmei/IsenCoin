#include <cmath>
#include <iostream>

#include <msgpack11/msgpack11.hpp>
#include <tacopie/tacopie>
#include <vili/Vili.hpp>

#include <argh.h>
#include <Chain.hpp>
#include <Config.hpp>
#include <Logger.hpp>
#include <Functions.hpp>
#include <Message.hpp>
#include <P2P.hpp>
#include <Tracker.hpp>
#include <Transaction.hpp>
#include <Ui.hpp>
#include <Utils.hpp>

#include <ed25519/fixedint.h>
#include <base58/base58.hpp>
#include "ed25519/ed25519.h"


std::condition_variable cv;

void
signint_handler(int) {
  cv.notify_all();
}

using namespace ic;

bool ok_log = ic::initialize_logger();

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

    unsigned int port;
    if (cmdl("port"))
    {
        try
        {
            port = std::stoi(cmdl("port").str());
        }
        catch (const std::invalid_argument& e)
        {
            Log->warn("Invalid port : \"{}\", using default port 15317 ({})", cmdl("port").str(), e.what());
            port = ic::config::DEFAULT_PORT;
        }
    }
    else
        port = ic::config::DEFAULT_PORT;

    Tracker tracker(port);
    vili::ViliParser config_file;
    config_file.parseFile("config.vili");
    for (const vili::DataNode* ip : config_file.at<vili::ArrayNode>("ips"))
        tracker.add_node(Node(ip->get<std::string>()));

    Chain::Initialize_Blockchain();

    ui::main(tracker);

    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);

    tacopie::close();

    return 0;
}
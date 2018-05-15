#pragma once

#include <msgpack11/msgpack11.hpp>
#include <tacopie/tacopie>

#include <Nodei.hpp>

namespace ic 
{
    namespace mp = msgpack11;

    class Tracker
    {
    private:
        tacopie::tcp_server m_server;
        std::vector<Node> m_nodes;
        std::vector<std::thread> m_connect_threads;
        uint16_t m_port;
    public:
        Tracker(unsigned int port = 15317);
        void add_node(Node node);
        void propagate(const mp::MsgPack& msg);
    };
}
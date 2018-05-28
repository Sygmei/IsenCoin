#pragma once

#include <msgpack11/msgpack11.hpp>
#include <tacopie/tacopie>

#include <Node.hpp>

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
        void start_server();
    public:
        Tracker(uint16_t port = 15317);
        void add_node(Node node);
        void propagate(const mp::MsgPack& msg);
        void handle_message(tacopie::tcp_client& client, const mp::MsgPack& msg);
        bool contains_node(Node node);
        std::vector<Node>& get_nodes();
        void set_port(uint16_t port);
        uint16_t get_port();
        void remove_node_at_index(unsigned int index);
        void send(const Node& node, const mp::MsgPack& msg) const;
    };
}
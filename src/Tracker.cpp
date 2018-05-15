#include <Tracker.hpp>

#include <Config.hpp>
#include <Logger.hpp>
#include <Message.hpp>
#include <P2P.hpp>

namespace ic 
{
    Tracker::Tracker(const unsigned int port)
    {
        Log->info("Starting Tracker using port {}", port);
        m_port = port;
        m_server.start("127.0.0.1", m_port, [&] (const std::shared_ptr<tacopie::tcp_client>& client) -> bool {
            Node node(client->get_socket().get_host(), client->get_socket().get_port());
            Log->info("(Server) New client connected {}:{}", node.get_address(), node.get_port());
            mp::MsgPack msg = p2p::build_msg("hello");
            p2p::send_msg(client->get_socket(), msg);
            msg = p2p::recv_msg(client->get_socket(), 200);
            p2p::use_msg(msg, "hello_too", [&](const auto& msg) {
                m_nodes.emplace_back(node.get_address(), msg["port"].uint16_value());
                Log->info("(Server) Successfully connected to Node {}:{}", node.get_address(), msg["port"].uint16_value());
            }, [&]() {
                Log->warn("(Server) New client {}:{} did not send correct handshake response : {}", node.get_address(), node.get_port(), msg.dump());
            }, {{ "port", mp::MsgPack::Type::UINT16 }});
            return true;
        });
    }

    void Tracker::add_node(Node node)
    {
        Log->info("(Client) Trying to add Node {}:{}...", node.get_address(), node.get_port());
        std::thread t([&](Node node) {
            tacopie::tcp_client client;
            mp::MsgPack msg;
            try
            {
                client.connect(node.get_address(), node.get_port(), 5000);
                Log->info("(Client) Connected to server {}:{}", node.get_address(), node.get_port());
                msg = p2p::recv_msg(client.get_socket(), 200);
            }
            catch (const std::exception& e)
            {
                Log->error("Failed to add Node {}:{} (Reason : {})", node.get_address(), node.get_port(), e.what());
            }
            p2p::use_msg(msg, "hello", [&](const auto& msg) {
                Log->info("(Client) Added Node {}:{} successfully", node.get_address(), node.get_port());
                m_nodes.push_back(node);
                mp::MsgPack msg_back = p2p::build_msg("hello_too", {{"port", m_port}});
                p2p::send_msg(client.get_socket(), msg_back);
            }, [&](){
                Log->warn("(Client) Rejected Node {}:{}, incorrect Handshake message : {}", node.get_address(), node.get_port(), msg.dump());
            });
        }, node);
        t.detach();
    }

    void Tracker::propagate(const mp::MsgPack& msg)
    {
        tacopie::tcp_client client;
        for (const auto& node : m_nodes)
        {
            try
            {
                client.connect(node.get_address(), node.get_port(), 1000);
                Log->info("(Client) Sending message to Node {}:{}", node.get_address(), node.get_port());
                p2p::send_msg(client.get_socket(), msg);
            }
            catch (const std::exception& e)
            {
                // REMOVE STALLED NODE
                Log->error("Failed to connect to {}:{} (Reason : {})", node.get_address(), node.get_port(), e.what());
            }
        }
    }
}
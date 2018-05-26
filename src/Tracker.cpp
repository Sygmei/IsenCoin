#include <Tracker.hpp>

#include <Chain.hpp>
#include <Config.hpp>
#include <Logger.hpp>
#include <Message.hpp>
#include <P2P.hpp>
#include <Transaction.hpp>
#include "base58/base58.hpp"

namespace ic 
{
    void Tracker::start_server()
    {
        if (m_server.is_running())
            m_server.stop(true, true);
        m_server.start(config::DEFAULT_BIND, m_port, [&](const std::shared_ptr<tacopie::tcp_client>& client) -> bool {
            Node node(client->get_socket().get_host(), client->get_socket().get_port());
            Log->info("(Server) New client connected {}:{}", node.get_address(), node.get_port());
            mp::MsgPack msg = p2p::recv_msg(client->get_socket(), 4096);
            this->handle_message(*client.get(), msg);
            client->disconnect();
            return true;
        });
    }

    Tracker::Tracker(uint16_t port)
    {
        Log->info("Starting Tracker using port {}", port);
        m_port = port;
        start_server();
    }

    void Tracker::add_node(Node node)
    {
        Log->info("(Client) Trying to add Node {}:{}...", node.get_address(), node.get_port());
        if (!contains_node(node))
        {
            std::thread t([&](Node node) {
                tacopie::tcp_client client;
                mp::MsgPack msg;
                try
                {
                    client.connect(node.get_address(), node.get_port(), 100000);
                    Log->info("(Client) Connected to server {}:{}", node.get_address(), node.get_port());
                }
                catch (const std::exception& e)
                {
                    Log->error("Failed to connect to Node {}:{} (Reason : {})", node.get_address(), node.get_port(), e.what());
                    return;
                }
                msg = p2p::build_msg("hello", { { "port", m_port } });
                p2p::send_msg(client.get_socket(), msg);
                msg = p2p::recv_msg(client.get_socket(), 200);
                p2p::use_msg(msg, "hello", [&](const auto& msg) {
                    if (msg["port"].uint16_value() == node.get_port())
                    {
                        Log->info("(Client) Added Node {}:{} successfully", node.get_address(), node.get_port());
                        if (!contains_node(node))
                            m_nodes.push_back(node);
                    }
                }, [&]() {
                    Log->warn("(Client) Rejected Node {}:{}, incorrect Handshake message : {}", node.get_address(), node.get_port(), msg.dump());
                });
                client.disconnect();
            }, node);
            t.detach();
        }
        else
        {
            Log->warn("Tracker already knows Node {}:{}, ignoring add_node()...", node.get_address(), node.get_port());
        }
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
                Log->debug("Msg Dump (s) : {}", msg.dump());
                p2p::send_msg(client.get_socket(), msg);
                client.disconnect();
            }
            catch (const std::exception& e)
            {
                Log->error("Failed to connect to {}:{} (Reason : {})", node.get_address(), node.get_port(), e.what());
                m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), node), m_nodes.end());
            }
        }
    }

    void Tracker::handle_message(tacopie::tcp_client& client, const mp::MsgPack& msg)
    {
        static uint32_t active_mining_thread = 0;
        Log->debug("Msg Dump (r) : {}", msg.dump());
        // @On "Hello" Message
        p2p::use_msg(msg, "hello", [&](const auto& msg) {
            if (!contains_node(Node(client.get_host(), msg["port"].uint16_value())))
            {
                mp::MsgPack msg_back = p2p::build_msg("hello", { { "port", m_port } });
                p2p::send_msg(client.get_socket(), msg_back);
                m_nodes.emplace_back(client.get_host(), msg["port"].uint16_value());
                Log->info("(Server) Successfully connected to Node {}:{}", client.get_host(), msg["port"].uint16_value());
            }
        }, [&]() {
            Log->warn("(Server) New client {}:{} did not send correct handshake response : {}", client.get_host(), client.get_port(), msg.dump());
        }, { { "port", mp::MsgPack::Type::UINT16 } });
        // @On "Transaction" Message
        p2p::use_msg(msg, "transaction", [&](const auto& msg)
        {
            std::vector<unsigned char> t_sender;
            base58::decode(msg["sender"].string_value(), t_sender);
            std::vector<unsigned char> t_receiver;
            base58::decode(msg["receiver"].string_value(), t_receiver);
            amount_t f_amount = msg["amount"].float32_value();
            timestamp_t f_timestamp = msg["timestamp"].uint64_value();
            std::vector<unsigned char> t_signature;
            base58::decode(msg["signature"].string_value(), t_signature);
            public_key_t f_sender;
            std::copy(t_sender.begin(), t_sender.end(), f_sender.begin());
            public_key_t f_receiver;
            std::copy(t_receiver.begin(), t_receiver.end(), f_receiver.begin());
            signature_t f_signature;
            std::copy(t_signature.begin(), t_signature.end(), f_signature.begin());
            Transaction tx(f_sender, f_receiver, f_amount, f_timestamp, f_signature);
            Log->debug("Validating Peer Transaction...");
            tx.validate();
            Chain::Blockchain().get_current_block().add_transaction(tx);
            Chain::Blockchain().mine_and_next();
        }, [&]()
        {
            Log->warn("(Server) Received invalid Transaction from {}:{} : '{}'", client.get_host(), client.get_port(), msg.dump());
        }, Transaction::Fields);
        // @On "AddPeer" Message
        p2p::use_msg(msg, "peer", [&](const auto& msg)
        {
            Node new_peer(msg["host"].string_value(), msg["port"].uint16_value());
            this->add_node(new_peer);
        }, [&]()
        {
            Log->warn("(Server) Received invalid Peer from {}:{} : '{}'", client.get_host(), client.get_port(), msg.dump());
        }, Node::Fields);
    }

    bool Tracker::contains_node(Node node)
    {
        return (std::find_if(m_nodes.begin(), m_nodes.end(), [&](const Node& p_node)
        {
            return (node.get_address() == p_node.get_address() && node.get_port() == p_node.get_port());
        }) != m_nodes.end());
    }

    std::vector<Node>& Tracker::get_nodes()
    {
        return m_nodes;
    }

    void Tracker::set_port(uint16_t port)
    {
        m_port = port;
        start_server();
    }

    void Tracker::remove_node_at_index(unsigned int index)
    {
        m_nodes.erase(m_nodes.begin() + index);
    }
}

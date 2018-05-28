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
                    client.connect(node.get_address(), node.get_port(), 10000);
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
            Transaction tx(msg);
            Log->warn("Just received transaction : {} ==> {}", tx.as_string(), msg.dump());
            tx.validate();
            Chain::Blockchain().get_current_block().add_transaction(tx);
            Chain::Blockchain().mine_and_next(*this);
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
        p2p::use_msg(msg, "block", [&](const auto& msg)
        {
            Log->warn("(Server) Received valid block ! {}", msg.dump());
            signature_t previous_hash;
            p2p::decode_b58(msg["previous_hash"].string_value(), previous_hash);
            std::vector<Transaction> txs;
            for (const auto& tx : msg["txs"].array_items())
            {
                txs.emplace_back(tx);
            }
            Block new_blk(msg["depth"].uint64_value(), previous_hash, msg["nonce"].uint64_value(), msg["timestamp"].uint64_value(), txs);
            if (new_blk.get_depth() == Chain::Blockchain().get_blockchain_size())
            {
                Chain::Blockchain().set_new_block(new_blk);
                if (msg["asked"].bool_value())
                {
                    p2p::send_msg(client.get_socket(), Chain::Blockchain().get_blockchain_size());
                }
            }
            else
                Log->error("Incompatible Block Depth '{}' with Local Blockchain size : {}", new_blk.get_depth(), Chain::Blockchain().get_blockchain_size());
        }, [&]()
        {
            Log->warn("(Server) Receiver invalid Block from {}:{} : '{}' (Size : {})", client.get_host(), client.get_port(), msg.dump(), msg.dump().size());
        }, Block::Fields);
        p2p::use_msg(msg, "ask_block", [&](const auto& msg)
        {
            uint64_t asked_depth = msg["depth"].uint64_value();
            if (asked_depth < Chain::Blockchain().get_blockchain_size())
            {
                mp::MsgPack msg_back = Chain::Blockchain().get_block_at_index(asked_depth).to_msgpack(true);
                Log->info("Asked block {} found in Blockchain, sending.. {}", asked_depth, msg_back.dump());
                send(Node(client.get_host(), client.get_port()), msg_back);
            }
            else
            {
                Log->error("Invalid AskBlock Index {} VS Blockchain size {}", asked_depth, Chain::Blockchain().get_blockchain_size());
                mp::MsgPack msg_back = p2p::build_msg("no_block");
                p2p::send_msg(client.get_socket(), msg_back);
            }
        }, [&]()
        {
            Log->error("(Server) Receiver invalid AskBlock from {}:{} : '{}' (Size : {})", client.get_host(), client.get_port(), msg.dump(), msg.dump().size());
        }, { { "depth", mp::MsgPack::Type::UINT64 } });
        p2p::use_msg(msg, "no_block", [&](const auto& msg)
        {
            Log->info("No more Block to update");
        }, [&]()
        {
            Log->error("(Server) Receiver invalid NoBlock from {}:{} : '{}' (Size : {})", client.get_host(), client.get_port(), msg.dump(), msg.dump().size());
        });
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

    uint16_t Tracker::get_port()
    {
        return m_port;
    }

    void Tracker::remove_node_at_index(unsigned int index)
    {
        m_nodes.erase(m_nodes.begin() + index);
    }

    void Tracker::send(const Node& node, const mp::MsgPack& msg) const
    {
        tacopie::tcp_client client;
        client.connect(node.get_address(), node.get_port(), 1000);
        p2p::send_msg(client.get_socket(), msg);
    }
}

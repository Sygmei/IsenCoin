#include <fmt/format.h>
#include <msgpack11/msgpack11.hpp>

#include <Functions.hpp>
#include <Logger.hpp>
#include <Node.hpp>
#include <Exceptions.hpp>

namespace ic 
{
    namespace mp = msgpack11;
    p2p::Requirements Node::Fields = {
        { "host", mp::MsgPack::Type::STRING },
        { "port", mp::MsgPack::Type::UINT16 },
    };

    bool Node::is_address_valid() const
    {
        std::vector<std::string> splitted_ip = utils::split(m_address, ".");
        if (splitted_ip.size() != 4) return false;
        for (const std::string& ip_segment : splitted_ip)
        {
            if (!utils::is_string_numeric(ip_segment) || std::stoi(ip_segment) < 0 || std::stoi(ip_segment) > 255)
                return false;
        }
        return true;
    }
    
    Node::Node(const std::string& address, const unsigned int port)
    {
        m_address = address;
        m_port = port;
        if (!is_address_valid())
            throw except::InvalidIpAddressException(m_address + std::to_string(m_port));
    }

    Node::Node(const std::string& address_and_port)
    {
        std::vector<std::string> splitted_ip = utils::split(address_and_port, ":");
        m_address = splitted_ip[0];
        if (!utils::is_string_numeric(splitted_ip[1]))
            throw except::InvalidIpAddressException(address_and_port);
        m_port = std::stoi(splitted_ip[1]);
        if (!is_address_valid())
            throw except::InvalidIpAddressException(address_and_port);
    }

    bool Node::operator==(const Node& node) const
    {
        return (m_address == m_address && m_port == m_port);
    }

    const std::string& Node::get_address() const
    {
        return m_address;
    }

    const unsigned int Node::get_port() const
    {
        return m_port;
    }
}
#pragma once

#include <stdexcept>
#include <string>

#include <P2P.hpp>

namespace ic 
{
    class Node 
    {
    private:
        std::string m_address;
        uint16_t m_port;
        bool is_address_valid() const;
    public:
        Node(const std::string& address, unsigned int port);
        Node(const std::string& address_and_port);
        bool operator==(const Node& node) const;
        const std::string& get_address() const;
        const unsigned int get_port() const;
        static p2p::Requirements Fields;
    };
}

#pragma once

#include <msgpack11/msgpack11.hpp>

class Message
{
private:
    msgpack11::MsgPack m_msgpack;
public:
    Message(const msgpack11::MsgPack& msgpack)
    {
        m_msgpack = msgpack;
    }
    Message(const msgpack11::MsgPack::object& msgpack)
    {
        m_msgpack = msgpack;
    }
    Message(const std::vector<char>& msg)
    {
        std::string msg_str(msg.begin(), msg.end());
        std::string err;
        m_msgpack = msgpack11::MsgPack::parse(msg_str, err);
    }
    const msgpack11::MsgPack& get_msg_pack()
    {
        return m_msgpack;
    }
    operator std::vector<char>() 
    {
        const std::string& msg_str = m_msgpack.dump();
        return std::vector<char>(msg_str.begin(), msg_str.end());
    }
    size_t size()
    {
        return m_msgpack.dump().size();
    }
};
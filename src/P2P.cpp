#include <Config.hpp>
#include <Exceptions.hpp>
#include <P2P.hpp>

#include <fmt/format.h>

namespace ic::p2p
{
    namespace mp = msgpack11;

    bool Requirement::has_check() const
    {
        return check.has_value();
    }

    bool Requirement::check_if_needed(const mp::MsgPack& msg)
    {
        if (this->has_check())
            return check.value()(msg);
        else
            return true;
    }

    Requirement::Requirement(const std::string& name, mp::MsgPack::Type type)
    {
        this->name = name;
        this->type = type;
    }

    Requirement::Requirement(const std::string& name, mp::MsgPack::Type type, additional_check_t check)
    {
        this->name = name;
        this->type = type;
        this->check = check;
    }

    mp::MsgPack string_to_msgpack(const std::string& msg)
    {
        std::string err;
        mp::MsgPack msgp = mp::MsgPack::parse(msg, err);
        return msgp;
    }

    mp::MsgPack bytearray_to_msgpack(const std::vector<char>& msg)
    {
        std::string msg_str(msg.begin(), msg.end());
        return string_to_msgpack(std::move(msg_str));
    }

    std::vector<char> string_to_bytearray(const std::string& msg)
    {
        std::vector<char> bytearray(msg.begin(), msg.end());
        return bytearray;
    }

    bool is_msgpack_valid(const mp::MsgPack& msg)
    {
        if (msg["ic"].is_bool() && msg["ic"] == true)
        {
            if (msg["isencoin"].is_uint8() && msg["isencoin"].int_value() == config::ISENCOIN_VERSION)
                return true;
            else
                throw except::VersionMismatchException(msg["isencoin"].int_value());
        }
        else
            throw except::NotIsenCoinMessageException(msg.dump());
    }

    bool is_msgpack_valid_type(const mp::MsgPack& msg, const std::string& type)
    {
        if (msg["type"].is_string() && msg["type"].string_value() == type)
            return true;
        else
            throw except::TypeMismatchException(type, msg["type"].string_value());
    }

    bool check_requirement(const mp::MsgPack& msg, Requirement req)
    {
        return (req.type == msg.type() && req.check_if_needed(msg));
    }

    void use_msg(
        const mp::MsgPack& msg, 
        const std::string& type, 
        success_callback_t on_success, 
        failure_callback_t on_failure,
        std::vector<Requirement> requirements
    )
    {
        try 
        {
            if (is_msgpack_valid(msg) && is_msgpack_valid_type(msg, type))
            {
                for (const auto& requirement : requirements)
                    check_requirement(msg[requirement.name], requirement);
                on_success(msg);
            }
        }
        catch (const except::Exception& e)
        {
            on_failure();
        }
    }

    mp::MsgPack build_msg(const std::string& type, mp::MsgPack::object fields)
    {
        mp::MsgPack::object headers {
            {"ic", true}, 
            {"isencoin", config::ISENCOIN_VERSION}, 
            {"type", type}
        };
        fields.insert(headers.begin(), headers.end());
        mp::MsgPack msg = fields;
        return msg;
    }

    void send_msg(tacopie::tcp_socket& socket, const mp::MsgPack& msg)
    {
        std::vector<char> byte_msg = p2p::string_to_bytearray(msg.dump());
        socket.send(byte_msg, byte_msg.size());
    }

    mp::MsgPack recv_msg(tacopie::tcp_socket& socket, size_t max_size)
    {
        return p2p::bytearray_to_msgpack(socket.recv(200));
    }
}
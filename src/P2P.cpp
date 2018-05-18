#include <Config.hpp>
#include <Exceptions.hpp>
#include <P2P.hpp>

#include <fmt/format.h>
#include <tacopie/utils/logger.hpp>

#include <iostream>

int tacopie::init()
{
    tacopie::active_logger = std::make_unique<tacopie::logger>();
    #ifdef _WIN32
        const WORD version = MAKEWORD(2, 2);
        WSADATA data;

        if (WSAStartup(version, &data) != 0) {
            std::cerr << "WSAStartup() failure" << std::endl;
            return -1;
        }
    #endif
    return 0;
}

void tacopie::close()
{
    #ifdef _WIN32
        WSACleanup();
    #endif
}

namespace ic::p2p
{
    namespace mp = msgpack11;

    std::string msgpack_type_to_string(mp::MsgPack::Type type)
    {
        switch (type)
        {
        case msgpack11::MsgPack::NUL: return "NUL";
        case msgpack11::MsgPack::FLOAT32: return "FLOAT32";
        case msgpack11::MsgPack::FLOAT64: return "FLOAT64";
        case msgpack11::MsgPack::INT8: return "INT8";
        case msgpack11::MsgPack::INT16: return "INT16";
        case msgpack11::MsgPack::INT32: return "INT32";
        case msgpack11::MsgPack::INT64: return "INT64";
        case msgpack11::MsgPack::UINT8: return "UINT8";
        case msgpack11::MsgPack::UINT16: return "UINT16";
        case msgpack11::MsgPack::UINT32: return "UINT32";
        case msgpack11::MsgPack::UINT64: return "UINT64";
        case msgpack11::MsgPack::BOOL: return "BOOL";
        case msgpack11::MsgPack::STRING: return "STRING";
        case msgpack11::MsgPack::BINARY: return "BINARY";
        case msgpack11::MsgPack::ARRAY: return "ARRAY";
        case msgpack11::MsgPack::OBJECT: return "OBJECT";
        case msgpack11::MsgPack::EXTENSION: return "EXTENSION";
        default: return "ERROR-TYPE";
        }
    }

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
        Log->trace("Compare {} and {}", type, msg["type"].string_value());
        return msg["type"].is_string() && msg["type"].string_value() == type;
    }

    bool check_requirement(const std::string& name, const mp::MsgPack& msg, Requirement req)
    {
        if (req.type != msg.type())
            throw except::MessageRequirementException(name, msgpack_type_to_string(req.type), msgpack_type_to_string(msg.type()));
        if (!req.check_if_needed(msg))
            throw except::MessageRequirementCustomCheckException(name);
    }

    void use_msg(
        const mp::MsgPack& msg, 
        const std::string& type, 
        success_callback_t& on_success, 
        failure_callback_t& on_failure,
        Requirements requirements
    )
    {
        try 
        {
            if (is_msgpack_valid(msg) && is_msgpack_valid_type(msg, type))
            {
                for (const auto& requirement : requirements)
                    check_requirement(requirement.name, msg[requirement.name], requirement);
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
        return p2p::bytearray_to_msgpack(socket.recv(max_size));
    }
}
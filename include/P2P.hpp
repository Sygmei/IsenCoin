#pragma once

#include <Logger.hpp>

#include <functional>
#include <optional>

#include <msgpack11/msgpack11.hpp>
#include <tacopie/network/tcp_socket.hpp>
#include "base58/base58.hpp"

namespace tacopie
{
    int init();
    void close();
}

namespace ic::p2p
{
    namespace mp = msgpack11;
    std::string msgpack_type_to_string(mp::MsgPack::Type type);
    using additional_check_t = std::function<bool(const mp::MsgPack&)>;
    struct Requirement
    {
        std::string name;
        mp::MsgPack::Type type;
        std::optional<additional_check_t> check;
        bool has_check() const;
        bool check_if_needed(const mp::MsgPack& msg);
        Requirement(const std::string& name, mp::MsgPack::Type type);
        Requirement(const std::string& name, mp::MsgPack::Type type, additional_check_t check);
    };
    using Requirements = std::vector<Requirement>;

    mp::MsgPack string_to_msgpack(const std::string& msg);
    mp::MsgPack bytearray_to_msgpack(const std::vector<char>& msg);
    std::vector<char> string_to_bytearray(const std::string& msg);
    bool is_msgpack_valid(const mp::MsgPack& msg);
    bool is_msgpack_valid_type(const mp::MsgPack& msg, const std::string& type);
    using success_callback_t = const std::function<void(const mp::MsgPack&)>;
    using failure_callback_t = const std::function<void()>;
    bool check_requirement(const std::string& name, const mp::MsgPack& msg, Requirement req);
    void use_msg(
        const mp::MsgPack& msg, 
        const std::string& type, 
        success_callback_t& on_success, 
        failure_callback_t& on_failure,
        Requirements requirements = {}
    );
    mp::MsgPack build_msg(const std::string& type, mp::MsgPack::object fields = {});
    void send_msg(tacopie::tcp_socket& socket, const mp::MsgPack& msg);
    mp::MsgPack recv_msg(tacopie::tcp_socket& socket, size_t max_size);
    template <size_t N>
    void decode_b58(const std::string& message, std::array<unsigned char, N>& tarray);

    template <size_t N>
    void decode_b58(const std::string& message, std::array<unsigned char, N>& tarray)
    {
        std::vector<unsigned char> buffer;
        base58::decode(message.c_str(), buffer);
        std::copy(buffer.begin(), buffer.end(), tarray.begin());
    }
}
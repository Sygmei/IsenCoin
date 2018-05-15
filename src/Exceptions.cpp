#include <Config.hpp>
#include <Exceptions.hpp>
#include <Logger.hpp>

#include <fmt/format.h>

namespace ic::except
{
    Exception::Exception(const std::string& message)
    {
        m_message = message;
        Log->error("{}", m_message);
    }

    const char* Exception::what() const noexcept
    {
        return m_message.c_str();
    }

    InvalidIpAddressException::InvalidIpAddressException(const std::string& address) : 
    Exception(fmt::format("Invalid ip address : {}", address)) {}

    VersionMismatchException::VersionMismatchException(uint16_t version) :
    Exception(fmt::format("IsenCoin Version Mismatch (Local:v{} / Remote:v{})", config::ISENCOIN_VERSION, version)) {}

    NotIsenCoinMessageException::NotIsenCoinMessageException(const std::string& msg) :
    Exception(fmt::format("Not an IsenCoin Message : {}", msg)) {}

    TypeMismatchException::TypeMismatchException(const std::string& expected_type, const std::string& real_type) :
    Exception(fmt::format("IsenCoin Message Type Mismatch (Expected : '{}', Got : '{}'", expected_type, real_type)) {}
}
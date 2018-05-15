#pragma once

#include <stdexcept>

namespace ic::except
{
    class Exception : public std::exception
    {
    protected:
        std::string m_message;
        Exception(const std::string& message);
    public:
        const char* what() const noexcept override;
    };

    struct InvalidIpAddressException : public Exception
    { InvalidIpAddressException(const std::string& address); };

    struct VersionMismatchException : public Exception
    { VersionMismatchException(uint16_t version); };

    struct NotIsenCoinMessageException : public Exception
    { NotIsenCoinMessageException(const std::string& msg); };

    struct TypeMismatchException : public Exception
    { TypeMismatchException(const std::string& expected_type, const std::string& real_type); };
}
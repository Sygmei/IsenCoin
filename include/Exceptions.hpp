#pragma once

#include <stdexcept>
#include "msgpack11/msgpack11.hpp"

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

    struct MessageRequirementException : public Exception
    { MessageRequirementException(const std::string& name, const std::string& expected_type, const std::string& real_type); };

    struct MessageRequirementCustomCheckException : public Exception
    { MessageRequirementCustomCheckException(const std::string& name); };

    struct InvalidTransactionException : public Exception
    { InvalidTransactionException(const std::string& tx_repr); };

    struct InvalidRewardAmount : public Exception
    { InvalidRewardAmount(float amount); };
}

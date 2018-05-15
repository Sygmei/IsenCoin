#pragma once

#include <string>
#include <vector>

namespace base58
{
    /**
     * Encode a byte sequence as a base58-encoded string.
     * pbegin and pend cannot be NULL, unless both are.
     */
    std::string encode(const unsigned char* pbegin, const unsigned char* pend);

    /**
     * Encode a byte vector as a base58-encoded string
     */
    std::string encode(const std::vector<unsigned char>& vch);

    /**
     * Decode a base58-encoded string (psz) into a byte vector (vchRet).
     * return true if decoding is successful.
     * psz cannot be NULL.
     */
    bool decode(const char* psz, std::vector<unsigned char>& vchRet);

    /**
     * Decode a base58-encoded string (str) into a byte vector (vchRet).
     * return true if decoding is successful.
     */
    bool decode(const std::string& str, std::vector<unsigned char>& vchRet);
}

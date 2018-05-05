#pragma once

#include <string>
#include <sstream>
#include <iomanip>

template <class T>
std::string char_array_to_hex(const T& array)
{
    std::stringstream ss;
    for (const auto& e : array)
        ss << std::hex << std::setfill('0') << std::setw(2)<< static_cast<unsigned int>(e);
    return ss.str();
}
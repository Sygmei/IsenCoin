#pragma once

#include <string>
#include <vector>

namespace ic::utils
{
    std::vector<std::string> split(const std::string& str, const std::string& delimiters);
    bool is_string_numeric(const std::string& str);
}
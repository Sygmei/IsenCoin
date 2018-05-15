#include <Functions.hpp>

#include <algorithm>

namespace ic::utils
{
    std::vector<std::string> split(const std::string& str, const std::string& delimiters)
    {
        std::vector<std::string> tokens;
        std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
        std::string::size_type pos = str.find_first_of(delimiters, lastPos);
        while (std::string::npos != pos || std::string::npos != lastPos)
        {
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            lastPos = str.find_first_not_of(delimiters, pos);
            pos = str.find_first_of(delimiters, lastPos);
        }
        return tokens;
    }

    bool is_string_numeric(const std::string& str)
    {
        if (!str.empty())
        {
            if (str.substr(0, 1) == "-")
                return all_of(str.substr(1).begin(), str.substr(1).end(), isdigit);
            return all_of(str.begin(), str.end(), isdigit);
        }
        return false;
    }
}
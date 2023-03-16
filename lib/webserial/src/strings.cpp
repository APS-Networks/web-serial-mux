#include "strings.hpp"

#include <string>
#include <vector>


auto smux::split(std::string const & to_split, char delim)
    -> std::vector<std::string>
{
    auto last = 0ull, next = 0ull;
    auto result = std::vector<std::string>{};
    while ((next = to_split.find(delim, last)) != std::string::npos) {
        result.emplace_back(to_split.substr(last, next - last));
        last = next + 1; 
    }
    result.emplace_back(to_split.substr(last));
    return result;
}

#pragma once

#include <string>
#include <vector>

namespace smux {

auto split(std::string const & to_split, char delim = ' ')
    -> std::vector<std::string>;

}
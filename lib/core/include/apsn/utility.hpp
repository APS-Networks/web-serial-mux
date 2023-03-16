#pragma once

#include <apsn/result.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace apsn::util {

auto read_all(fs::path path) -> apsn::result<std::string>;

}
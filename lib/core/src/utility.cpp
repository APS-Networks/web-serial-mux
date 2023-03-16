#include "utility.hpp"

#include <apsn/result.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;


auto apsn::util::read_all(fs::path path) -> apsn::result<std::string>
{
    using namespace std::string_literals;

    if (!fs::exists(path)) {
        return std::make_error_code(std::errc::no_such_file_or_directory);
    }
    auto stream = std::ifstream{path};
    auto content = std::string{
            std::istreambuf_iterator<char>{stream},
            std::istreambuf_iterator<char>{},
        };
    
    return content;
}
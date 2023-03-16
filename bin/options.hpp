#pragma once

#include <apsn/logging.hpp>


#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace fs = std::filesystem;


struct options
{
    options()
        : host{"0.0.0.0"}
        , pass{}
        , port{8080}
        , log_level{apsn::log::level::info}
        , root{fs::current_path()}
    {}
    std::string host;
    fs::path pass;
    std::uint16_t port;
    apsn::log::level log_level;
    fs::path root;
    std::optional<fs::path> cert_path;
    std::optional<fs::path> key_path;
    std::optional<fs::path> dh_path;
};


auto get_options(int argc, char const * argv[]) -> options;

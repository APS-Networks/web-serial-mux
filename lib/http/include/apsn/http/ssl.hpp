#pragma once

#include <boost/asio/ssl.hpp>


#include <filesystem>
#include <memory>
#include <string>


namespace apsn::ssl {

namespace fs = std::filesystem;
namespace ssl = boost::asio::ssl;

auto make_context(std::string cert, std::string key, std::string dh)
    -> std::shared_ptr<ssl::context>;

auto make_context(
        fs::path const & cert_path, 
        fs::path const & key_path, 
        fs::path const & dh_path)
    -> std::shared_ptr<ssl::context>;

}
#include "ssl.hpp"

#include <apsn/logging.hpp>
#include <apsn/utility.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <filesystem>
#include <memory>
#include <string>


namespace fs = std::filesystem;
namespace asio = boost::asio;

auto apsn::ssl::make_context(std::string cert, std::string key, std::string dh)
    -> std::shared_ptr<ssl::context>
{
    auto ctx = std::make_shared<ssl::context>(ssl::context::tlsv12);
    ctx->set_password_callback(
        [](std::size_t, ssl::context_base::password_purpose)
        {
            return "";
        });
    ctx->set_options(ssl::context::default_workarounds |
                        ssl::context::no_sslv2 |
                        ssl::context::single_dh_use);
    ctx->use_certificate_chain(asio::buffer(cert.data(), cert.size()));
    ctx->use_private_key(asio::buffer(key.data(), key.size()),
            ssl::context::file_format::pem);

    ctx->use_tmp_dh(
        boost::asio::buffer(dh.data(), dh.size()));
    
    return ctx;
}

auto apsn::ssl::make_context(
        fs::path const & cert_path, 
        fs::path const & key_path, 
        fs::path const & dh_path)
    -> std::shared_ptr<ssl::context>
{
    auto cert_data = util::read_all(cert_path);
    auto key_data = util::read_all(key_path);
    auto dh_data = util::read_all(dh_path);

    if (!cert_data) {
        apsn::log::fatal("Error reading certificate {}: ",
                cert_path.string(),
                cert_data.error.message());
        return nullptr;
    }

    if (!key_data) {
        apsn::log::fatal("Error reading private key {}: ",
                key_path.string(),
                key_data.error.message());
        return nullptr;
    }

    if (!dh_data) {
        apsn::log::fatal("Error reading Diffie-Helmann parameters {}: ",
                dh_path.string(),
                dh_data.error.message());
        return nullptr;
    }

    apsn::log::debug("Using cert {}", cert_path.string());
    apsn::log::debug("Using key {}", key_path.string());
    apsn::log::debug("Using DH {}", dh_path.string());

    return make_context(*cert_data, *key_data, *dh_data);
}
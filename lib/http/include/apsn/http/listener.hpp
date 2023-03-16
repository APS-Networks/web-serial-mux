#pragma once

#include "session.hpp"

#include <apsn/logging.hpp>
#include <apsn/http/handlers.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>


namespace asio = boost::asio;
namespace beast = boost::beast;
namespace ip = asio::ip;
namespace sys = boost::system;
namespace ssl = asio::ssl;

using tcp = ip::tcp;

namespace apsn::http {

// template <typename State>
template <typename Traits>
class listener : public std::enable_shared_from_this<listener<Traits>>
{
    using shared_type = typename Traits::shared_type;
    using handler_type = typename Traits::handler_type;
public:
    listener(asio::io_context & ioc,
            tcp::endpoint endpoint,
            std::shared_ptr<shared_type> shared,
            std::shared_ptr<handler_type> handler);

    auto run() -> void;

private:
    auto fail(beast::error_code ec, char const* what) -> void;
    auto on_accept(beast::error_code ec, tcp::socket socket) -> void;

    asio::io_context & m_ioc;
    tcp::acceptor m_acceptor;
    std::shared_ptr<shared_type> m_shared;
    std::shared_ptr<handler_type> m_handler;
};



using ssl_ctx_ptr = std::shared_ptr<ssl::context>;


template <typename Traits>
class ssl_listener : public std::enable_shared_from_this<ssl_listener<Traits>>
{
    using shared_type = typename Traits::shared_type;
    using handler_type = typename Traits::handler_type;

public:
    ssl_listener(asio::io_context & ioc,
            tcp::endpoint endpoint,
            ssl_ctx_ptr ssl,
            std::shared_ptr<shared_type> shared,
            std::shared_ptr<handler_type> handler);

    auto run() -> void;

private:
    auto fail(beast::error_code ec, char const* what) -> void;
    auto on_accept(beast::error_code ec, tcp::socket socket) -> void;

    asio::io_context & m_ioc;
    tcp::acceptor m_acceptor;
    ssl_ctx_ptr m_ssl;
    std::shared_ptr<shared_type> m_shared;
    std::shared_ptr<handler_type> m_handler;

};

#include <apsn/http/detail/listener.tpp>

}
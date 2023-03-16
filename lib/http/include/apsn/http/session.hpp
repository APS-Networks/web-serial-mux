#pragma once


#include <apsn/logging.hpp>

#include <apsn/http/handlers.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>

#include <chrono>
#include <optional>


namespace asio = boost::asio;
namespace beast = boost::beast;
namespace ip = asio::ip;
namespace sys = boost::system;
namespace ssl = asio::ssl;

using tcp = ip::tcp;

namespace apsn::http {

template <typename Impl, typename Traits, bool IsSSL>
class session_base
{
public:
    using shared_type = typename Traits::shared_type;
    using unique_type = typename Traits::unique_type;
    using handler_type = typename Traits::handler_type;

    session_base(std::shared_ptr<shared_type> shared,
                 std::shared_ptr<handler_type> handler)
        : m_unique{std::make_shared<unique_type>()}
        , m_shared{shared}
        , m_handler{handler}
    {}

    auto cast() -> Impl&;

    void fail(sys::error_code, char const* what);
    void send(beast::http::message_generator &&);

    void on_read(sys::error_code, std::size_t);
    void on_write(bool close, sys::error_code, std::size_t);

    void do_read_header();
    void on_read_header(sys::error_code, std::size_t);

    auto on_ws_upgrade() -> std::optional<apsn::http::response>
    {
        using ws_type = typename Traits::websocket_handler_type<IsSSL>;
        
        return std::make_shared<ws_type>(
                    std::move(cast().stream()),
                    this->m_unique,
                    this->m_shared)
                ->run(this->m_parser->release());
    }

    auto buffer() -> beast::flat_buffer&
    { return m_buffer; }

    beast::flat_buffer m_buffer;


    std::optional<
        beast::http::request_parser<
            beast::http::empty_body>> m_parser;

    std::shared_ptr<unique_type> m_unique;
    std::shared_ptr<shared_type> m_shared;
    std::shared_ptr<handler_type> m_handler;
};



template <typename Traits>
class session
    : public session_base<session<Traits>, Traits, false>
    , public std::enable_shared_from_this<session<Traits>>
{
    using base_type = session_base<session<Traits>, Traits, false>;
    using shared_type = typename Traits::shared_type;
    using handler_type = typename Traits::handler_type;

public:
    session(tcp::socket&& socket, 
        std::shared_ptr<shared_type> shared,
        std::shared_ptr<handler_type> handler_);

    auto stream() -> beast::tcp_stream&
    { return m_stream; }

    auto do_eof() -> void;
    auto run() -> void;

private:
    beast::tcp_stream m_stream;

};


using ssl_ctx_ptr = std::shared_ptr<ssl::context>;


template <typename Traits>
class ssl_session
    : public session_base<ssl_session<Traits>, Traits, true>
    , public std::enable_shared_from_this<ssl_session<Traits>>
{
    using base_type = session_base<ssl_session<Traits>, Traits, true>;    
    using shared_type = typename Traits::shared_type;
    using handler_type = typename Traits::handler_type;

public:
    ssl_session(tcp::socket&& socket,
            ssl_ctx_ptr ssl,
            std::shared_ptr<shared_type> shared,
            std::shared_ptr<handler_type> handler);

    auto stream() -> beast::ssl_stream<beast::tcp_stream>&
    { return m_stream; }

    auto on_handshake(beast::error_code ec, std::size_t bytes_used);
    auto run() -> void;
    auto do_eof() -> void;
    auto on_shutdown(beast::error_code ec) -> void;

private:
    beast::ssl_stream<beast::tcp_stream> m_stream;
    ssl_ctx_ptr m_ssl_ctx;

};


#include <apsn/http/detail/session.tpp>


}
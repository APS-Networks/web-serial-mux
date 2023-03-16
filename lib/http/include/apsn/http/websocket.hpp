#pragma once


#include <apsn/logging.hpp>
#include <apsn/http/request.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <iosfwd>
#include <memory>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace sys = boost::system;

using ssl_ctx_ptr = std::shared_ptr<ssl::context>;

using tcp = boost::asio::ip::tcp;

template <typename Body, typename Alloc>
using beast_request = beast::http::request<Body,
        beast::http::basic_fields<Alloc>>;



namespace apsn::ws {


template <bool IsSSL>
struct streams_when;

template <>
struct streams_when<true>
{
    using ws_stream_type = websocket::stream<beast::ssl_stream<beast::tcp_stream>>;
    using tcp_stream_type = beast::ssl_stream<beast::tcp_stream>;
};

template <>
struct streams_when<false>
{
    using ws_stream_type = websocket::stream<beast::tcp_stream>;
    using tcp_stream_type = beast::tcp_stream;
};



template <typename Impl>
struct is_shared_from_this : std::conditional_t<
        std::is_base_of_v<std::enable_shared_from_this<Impl>, Impl>,
        std::true_type,
        std::false_type
    > {};

template <typename T>
constexpr static auto is_shared_from_this_v = is_shared_from_this<std::decay_t<T>>::value;


class websocket_base
{
public:
    virtual auto ostream() -> std::ostream & = 0;
    virtual auto cancel() -> void = 0;
};


template <typename HandlerImpl, typename Traits, bool IsSSL>
class websocket_impl : public websocket_base//<Traits>
{
    using self_type = websocket_impl<HandlerImpl, Traits, IsSSL>;
    using ws_stream_type = typename streams_when<IsSSL>::ws_stream_type;
public:
    using stream_type = typename streams_when<IsSSL>::tcp_stream_type;
    using shared_type = typename Traits::shared_type;
    using unique_type = typename Traits::unique_type;

    class streambuf : public std::streambuf
    {
    public:
        streambuf(self_type * session)
            : m_session{session}
        {}

        auto xsputn(char const * s, std::streamsize n) 
            -> std::streamsize override
        {
            m_session->send(std::make_shared<std::string>(s, s + n));
            return n;
        } 

        auto overflow(int c) -> int override
        {
            m_session->send(std::make_shared<std::string>(1,
                    static_cast<char>(c)));
            return 1;
        }

    private:
        self_type * m_session;
    };

    websocket_impl(stream_type && stream,
            std::shared_ptr<unique_type> unique,
            std::shared_ptr<shared_type> shared)
        : m_stream{std::move(stream)}
        , m_buffer{}
        , m_queue{}
        , m_streambuf{this}
        , m_ostream{&m_streambuf}
        , m_unique{unique}
        , m_shared{shared}
    {
        static_assert(is_shared_from_this_v<HandlerImpl>, 
                "Websocket handler must derive from shared_from_this");
    }


    template <typename Body, typename Alloc>
    auto run(beast_request<Body, Alloc> && req)
        -> std::optional<apsn::http::response>
    {
        auto error = handler_layer().handle_request(req);
        if (error) {
            return error;
        }
    
        beast::get_lowest_layer(m_stream).expires_never();

        m_stream.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));

        m_stream.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res) {
                res.set(beast::http::field::server, "apsn-serial-mux");
            }));

        m_stream.async_accept(
            req,
            beast::bind_front_handler(
                &self_type::on_accept,
                handler_layer().shared_from_this()));

        return {};
    }

    auto ostream() -> std::ostream &
    { return m_ostream; }

    auto shared() -> std::shared_ptr<shared_type>
    { return m_shared; }

    auto session() -> std::shared_ptr<unique_type>
    { return m_unique; }

    auto buffer() -> beast::flat_buffer &
    { return m_buffer; }

    auto stream() -> ws_stream_type &
    { return m_stream; }

    auto cancel() -> void override
    { 
        auto self = handler_layer().shared_from_this();
        return m_stream.async_close(websocket::close_code::normal,
                [self](sys::error_code) {
                    apsn::log::info("Websocket closed");
                });
    }

private:
    auto handler_layer() -> HandlerImpl&
    { return static_cast<HandlerImpl&>(*this); }

    auto send(std::shared_ptr<std::string const> const& ss) -> void;
    auto fail(beast::error_code ec, char const* what) -> void;
    auto on_accept(beast::error_code ec) -> void;
    auto on_read(beast::error_code ec, std::size_t bytes_transferred) -> void;
    auto on_send(std::shared_ptr<std::string const> const& ss) -> void;
    auto on_write(beast::error_code ec, std::size_t bytes_transferred) -> void;

    ws_stream_type m_stream;
    beast::flat_buffer m_buffer;
    std::vector<std::shared_ptr<std::string const>> m_queue;
    streambuf m_streambuf;
    std::ostream m_ostream;
    std::shared_ptr<unique_type> m_unique;
    std::shared_ptr<shared_type> m_shared;
};

#include <apsn/http/detail/websocket.tpp>

template <template <typename, bool> typename Handler,
        typename Traits, 
        bool IsSSL>
using websocket_handler = 
        websocket_impl<Handler<Traits, IsSSL>, Traits, IsSSL>;

}
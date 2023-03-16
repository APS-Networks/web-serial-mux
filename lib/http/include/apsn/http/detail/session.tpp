
#include <boost/beast/core/stream_traits.hpp>


template <typename Impl, typename Traits, bool IsSSL>
auto session_base<Impl, Traits, IsSSL>::cast() -> Impl&
{
    return static_cast<Impl&>(*this);
}


template <typename Impl, typename Traits, bool IsSSL>
auto session_base<Impl, Traits, IsSSL>::fail(beast::error_code ec, char const* what) -> void
{
    // if (ec == asio::error::operation_aborted || 
    //     ec == asio::error::timed_out || 
    //     ec == beast::error::timeout) {
    //     return;
    // }

    apsn::log::warn("session_base::fail {}: {}", what, ec.message());
}


template <typename Impl, typename Traits, bool IsSSL>
auto session_base<Impl, Traits, IsSSL>::do_read_header() -> void
{
    using namespace std::chrono_literals;

    beast::get_lowest_layer(cast().stream())
            .expires_after(30s);

    static_assert(boost::beast::is_sync_stream<
            std::decay_t<decltype(cast().stream())>
        >::value, "not a sync stream");


    m_parser.emplace();

    beast::http::async_read_header(
        cast().stream(),
        m_buffer,
        *m_parser,
        beast::bind_front_handler(
            &session_base::on_read_header,
            cast().shared_from_this()));
}


template <typename Impl, typename Traits, bool IsSSL>
auto session_base<Impl, Traits, IsSSL>::on_read_header(
        beast::error_code ec, 
        std::size_t transferred)
    -> void
{
    boost::ignore_unused(transferred);

    if (ec == beast::http::error::end_of_stream) {
        apsn::log::debug("End-of-stream");
        return cast().do_eof();
    }

    if (ec) {
        return fail(ec, "read");
    }

    auto source = beast::get_lowest_layer(cast().stream())
            .socket()
            .remote_endpoint()
            .address()
            .to_string();

    auto hdr_response = m_handler->before_body(source, *m_parser,
            m_parser->get());
    if (hdr_response) {
        return send(hdr_response->message());
    }

    /* TODO: Alter parser object here for body types and various verbs.
             Use std::variant<std::monostate, BodyTypes...> to hold the
             parser. Use move constructor on new parser from old. Potential
             use of std::visit for body type, but that would have to play
             fair with router and/or handlers. */
    
    beast::http::async_read(
        cast().stream(),
        m_buffer,
        *m_parser,
        beast::bind_front_handler(
            &session_base::on_read,
            cast().shared_from_this()));
}


template <typename Impl, typename Traits, bool IsSSL>
auto session_base<Impl, Traits, IsSSL>::on_read(
        beast::error_code ec, 
        std::size_t transferred)
    -> void
{
    boost::ignore_unused(transferred);

    if (ec == beast::http::error::end_of_stream) {
        return cast().do_eof();
    }

    if (ec) {
        return fail(ec, "read");
    }



    namespace websocket = beast::websocket;
    if (websocket::is_upgrade(m_parser->get())) {
        auto error = on_ws_upgrade();
        if (error) {
            return send(error->message());
        }
        return;
    }

    auto source = beast::get_lowest_layer(cast().stream())
            .socket()
            .remote_endpoint()
            .address()
            .to_string();

    send(m_handler->handle(source, std::move(m_parser->release())).message());
}



template <typename Impl, typename Traits, bool IsSSL>
auto session_base<Impl, Traits, IsSSL>::send(beast::http::message_generator && msg)
    -> void
{
    bool keep_alive = msg.keep_alive();

    beast::async_write(
        cast().stream(),
        std::move(msg),
        beast::bind_front_handler(
            &session_base::on_write,
            cast().shared_from_this(),
            keep_alive));
}


template <typename Impl, typename Traits, bool IsSSL>
auto session_base<Impl, Traits, IsSSL>::on_write(
        bool keep_alive,
        beast::error_code ec, 
        std::size_t transferred) 
    -> void
{
    boost::ignore_unused(transferred);

    if (ec) {
        return fail(ec, "write");
    }

    if (!keep_alive) {
        return cast().do_eof();
    }

    do_read_header();
}






using apsn::http::session;
using apsn::http::ssl_session;

template <typename Traits>
session<Traits>::session(
        tcp::socket&& socket,
        std::shared_ptr<shared_type> shared,
        std::shared_ptr<handler_type> handler_)
    : session_base<session, Traits, false>(shared, handler_)
    , m_stream(std::move(socket))
{
}


template <typename Traits>
auto session<Traits>::do_eof() -> void
{
    beast::error_code ec;
    m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
}


template <typename Traits>
auto session<Traits>::run() -> void
{
    asio::dispatch(m_stream.get_executor(),
        beast::bind_front_handler(
            &base_type::do_read_header,
            this->shared_from_this()));
}





template <typename Traits>
ssl_session<Traits>::ssl_session(
            tcp::socket&& socket, 
            ssl_ctx_ptr ssl_ctx, 
            std::shared_ptr<shared_type> shared,
            std::shared_ptr<handler_type> handler_)
    : session_base<ssl_session, Traits, true>(shared, handler_)
    , m_stream(std::move(socket), *ssl_ctx)
    , m_ssl_ctx{ssl_ctx}
{
}


template <typename Traits>
auto ssl_session<Traits>::on_handshake(beast::error_code ec, std::size_t bytes_used)
{
    if (ec)
        return this->fail(ec, "handshake");

    this->buffer().consume(bytes_used);
    this->do_read_header();
}


template <typename Traits>
auto ssl_session<Traits>::run() -> void
{
    using namespace std::chrono_literals;

    auto self = this->shared_from_this();
    asio::dispatch(m_stream.get_executor(),
        [self](){
            beast::get_lowest_layer(self->m_stream).expires_after(30s);
            self->m_stream.async_handshake(
                ssl::stream_base::server,
                self->buffer().data(),
                beast::bind_front_handler(
                    &ssl_session::on_handshake,
                    self
                )
            );
        });
}



template <typename Traits>
auto ssl_session<Traits>::do_eof() -> void
{
    using namespace std::chrono_literals;

    beast::get_lowest_layer(m_stream).expires_after(30s);

    m_stream.async_shutdown(
        beast::bind_front_handler(
            &ssl_session::on_shutdown,
            this->shared_from_this()));
}


template <typename Traits>
auto ssl_session<Traits>::on_shutdown(beast::error_code ec) -> void
{
    if (ec) {
        return this->fail(ec, "shutdown");
    }
}

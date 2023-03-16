

#define WS_IMPL_BASE websocket_impl<HandlerImpl, Traits, IsSSL>

template <typename HandlerImpl, typename Traits, bool IsSSL>
auto WS_IMPL_BASE::send(std::shared_ptr<std::string const> const& ss) -> void
{
    asio::post(
        m_stream.get_executor(),
        beast::bind_front_handler(
            &self_type::on_send,
            handler_layer().shared_from_this(),
            ss));
}


template <typename HandlerImpl, typename Traits, bool IsSSL>
auto WS_IMPL_BASE::fail(beast::error_code ec, char const* what) -> void
{
    apsn::log::error("{}: {}", what, ec.message());
}


template <typename HandlerImpl, typename Traits, bool IsSSL>
auto WS_IMPL_BASE::on_accept(beast::error_code ec) -> void
{
    if (ec)  {
        return fail(ec, "accept");
    }

    handler_layer().handle_accept();

    apsn::log::debug("Stream accepted");
    m_stream.async_read(
        m_buffer,
        beast::bind_front_handler(
            &self_type::on_read,
            handler_layer().shared_from_this()));
}


template <typename HandlerImpl, typename Traits, bool IsSSL>
auto WS_IMPL_BASE::on_read(beast::error_code ec, std::size_t bytes_transferred) -> void
{
    if (ec)  {
        return fail(ec, "read");
    }

    handler_layer().handle_message();


    m_buffer.consume(bytes_transferred);

    /* TODO:, do something with buffer */
    m_stream.async_read(
        m_buffer,
        beast::bind_front_handler(
            &self_type::on_read,
            handler_layer().shared_from_this()));
}


template <typename HandlerImpl, typename Traits, bool IsSSL>
auto WS_IMPL_BASE::on_send(std::shared_ptr<std::string const> const& ss) -> void
{
    m_queue.push_back(ss);

    // Are we already writing?
    if(m_queue.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    m_stream.async_write(
        asio::buffer(*m_queue.front()),
        beast::bind_front_handler(
            &self_type::on_write,
            handler_layer().shared_from_this()));
}


template <typename HandlerImpl, typename Traits, bool IsSSL>
auto WS_IMPL_BASE::on_write(beast::error_code ec, std::size_t bytes_transferred) -> void
{
    if(ec) {
        return fail(ec, "write");
    }

    apsn::log::trace("websocket_session: Wrote {} bytes", bytes_transferred);

    // Remove the string from the queue
    m_queue.erase(m_queue.begin());

    // Send the next message if any
    if (!m_queue.empty()) {
        m_stream.async_write(
            asio::buffer(*m_queue.front()),
            beast::bind_front_handler(
                &self_type::on_write,
                handler_layer().shared_from_this()));  
    }
}
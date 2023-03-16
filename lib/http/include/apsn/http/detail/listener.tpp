
template <typename Traits>
listener<Traits>::listener(asio::io_context & ioc, 
        tcp::endpoint endpoint, 
        std::shared_ptr<shared_type> shared,
        std::shared_ptr<handler_type> handler)
    : m_ioc{ioc}
    , m_acceptor(m_ioc)
    , m_shared{shared}
    , m_handler{handler}
{
    auto ec = sys::error_code{};

    m_acceptor.open(endpoint.protocol(), ec);
    if(ec) {
        fail(ec, "open");
        return;
    }

    m_acceptor.set_option(asio::socket_base::reuse_address(true), ec);
    if(ec) {
        fail(ec, "set_option");
        return;
    }

    m_acceptor.bind(endpoint, ec);
    if(ec) {
        fail(ec, "bind");
        return;
    }

    m_acceptor.listen(asio::socket_base::max_listen_connections, ec);
    if(ec) {
        fail(ec, "listen");
        return;
    }
}

template <typename Traits>
auto listener<Traits>::run() -> void 
{
    m_acceptor.async_accept(
        asio::make_strand(m_ioc),
        beast::bind_front_handler(
            &listener::on_accept,
            this->shared_from_this()));
}


template <typename Traits>
auto listener<Traits>::fail(beast::error_code ec, char const* what) -> void 
{
    if(ec == asio::error::operation_aborted)
        return;
    apsn::log::error("{}:{}", what, ec.message());
}


template <typename Traits>
auto listener<Traits>::on_accept(beast::error_code ec, tcp::socket socket) -> void 
{
    if(ec) {
        return this->fail(ec, "accept");
    }
    else {
        std::make_shared<session<Traits>>(std::move(socket), m_shared, m_handler)->run();
    }
    m_acceptor.async_accept(
        asio::make_strand(m_ioc),
        beast::bind_front_handler(
            &listener::on_accept,
            this->shared_from_this()));
}


template <typename Traits>
ssl_listener<Traits>::ssl_listener(asio::io_context & ioc, 
        tcp::endpoint endpoint, 
        ssl_ctx_ptr ssl,
        std::shared_ptr<shared_type> shared,
        std::shared_ptr<handler_type> handler)
    : m_ioc{ioc}
    , m_acceptor(m_ioc)
    , m_ssl{ssl}
    , m_shared{shared}
    , m_handler{handler}
{
    // apsn::log::debug("listener::listener");

    auto ec = sys::error_code{};

    m_acceptor.open(endpoint.protocol(), ec);
    if(ec) {
        fail(ec, "open");
        return;
    }

    m_acceptor.set_option(asio::socket_base::reuse_address(true), ec);
    if(ec) {
        fail(ec, "set_option");
        return;
    }

    m_acceptor.bind(endpoint, ec);
    if(ec) {
        fail(ec, "bind");
        return;
    }

    m_acceptor.listen(asio::socket_base::max_listen_connections, ec);
    if(ec) {
        fail(ec, "listen");
        return;
    }
}


template <typename Traits>
auto ssl_listener<Traits>::run() -> void 
{
    // apsn::log::debug("listener::run");

    m_acceptor.async_accept(
        asio::make_strand(m_ioc),
        beast::bind_front_handler(
            &ssl_listener::on_accept,
            this->shared_from_this()));
}


template <typename Traits>
auto ssl_listener<Traits>::fail(beast::error_code ec, char const* what) -> void 
{
    if(ec == asio::error::operation_aborted)
        return;
    apsn::log::error("{}:{}", what, ec.message());
}


template <typename Traits>
auto ssl_listener<Traits>::on_accept(
        beast::error_code ec, 
        tcp::socket socket) -> void 
{
    // apsn::log::debug("listener::on_accept");

    if (ec) {
        return fail(ec, "accept");
    }
    else {
        // apsn::log::info("accepted");

        std::make_shared<ssl_session<Traits>>(std::move(socket),
                m_ssl,
                m_shared,
                m_handler)->run();
    }

    m_acceptor.async_accept(
        asio::make_strand(m_ioc),
        beast::bind_front_handler(
            &ssl_listener::on_accept,
            this->shared_from_this()));
}
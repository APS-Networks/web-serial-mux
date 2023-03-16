template <typename Traits>
router<Traits>::router(std::shared_ptr<shared_type> shared)
    : m_shared{shared}
{}

template <typename Traits>
template <typename Alloc>
auto router<Traits>::before_body(std::string source, 
        beast_empty_parser<Alloc> & prsr,
        beast_empty_request<Alloc> & req)
    -> std::optional<response>
{
    /* Take request by reference */
    auto request = apsn::http::request<Traits>{source, req, m_shared};
    auto parser = apsn::http::basic_parser{prsr};

    switch (request.method()) {
    case boost::beast::http::verb::get: {
        return handle_get_before_body(parser, request);
    }
    default: break;
    }
    return std::nullopt;
}

template <typename Traits>
template <typename Body, typename Alloc>
auto router<Traits>::handle(std::string source, 
        beast_request<Body, Alloc> && req)-> response
{
    auto response = std::optional<beast::http::message_generator>{};
    auto request = apsn::http::request<Traits>{
            source,
            std::move(req),
            m_shared};

    switch (request.method()) {
    case boost::beast::http::verb::get:
        return handle_get(request);
    default: break;
    }

    return apsn::http::bad_request(request, "Unhandled");
}

template <typename Traits>
auto router<Traits>::handle_get(request<Traits> & request) -> response
{
    auto it = m_get.longest_match(std::string{request.target()});
    if (it == std::end(m_get)) {
        return bad_request(request, "Unhandled");
    }
    auto [type, ptr] = it->second;
    apsn::log::trace("match: {}", it->first);
    if (type == router_match::exact && request.target() != it->first) {
        return bad_request(request, "Rejected");
    }
    return ptr->handle(request);
}

template <typename Traits>
auto router<Traits>::handle_get_before_body(
        basic_parser & parser,
        request<Traits> & request)
    -> std::optional<response>
{
    auto it = m_get.longest_match(std::string{request.target()});
    if (it == std::end(m_get)) {
        return bad_request(request, "Unhandled");
    }
    auto [type, ptr] = it->second;
    apsn::log::trace("match: {}", it->first);
    if (type == router_match::exact && request.target() != it->first) {
        return bad_request(request, "Rejected");
    }
    return ptr->before_body(parser, request);
}

template <typename Traits>
template <typename F>
auto router<Traits>::get(std::string path, router_match type, F && func)
{
    insert(m_get, path, matcher{type,
        ensure_handler<Traits>(std::forward<F>(func))});
}

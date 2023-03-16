


template <typename Traits>
auto apsn::http::bad_request(request<Traits> & req, std::string_view why) -> response
{
    auto res = beast::http::response<beast::http::string_body>{
            beast::http::status::bad_request,
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
};


template <typename Traits>
auto apsn::http::not_found(request<Traits> & req) -> response
{
    auto res = beast::http::response<beast::http::string_body>{
            beast::http::status::not_found,
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + 
            std::string(req.target()) + 
            "' was not found.";
    res.prepare_payload();
    return res;
};


template <typename Traits>
auto apsn::http::server_error(request<Traits> & req, std::string_view what)
    -> response
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::internal_server_error, 
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
};


// template <typename Traits>
// auto apsn::http::unauthorised(request<Traits> & req, fs::path body) -> response
// {
//     beast::http::response<beast::http::file_body> res{
//             beast::http::status::unauthorized, 
//             req.version()};
//     res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
//     res.set(beast::http::field::content_type, "text/html");
//     res.keep_alive(req.keep_alive());
//     res.prepare_payload();
//     return res;
// }


template <typename Traits>
auto apsn::http::basic_auth(request<Traits> & req, std::string_view realm) -> response
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::unauthorized, 
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");

    auto settings = fmt::format("Basic realm=\"{}\", charset=\"utf-8\"", 
            realm);
    res.set("WWW-Authenticate", settings);
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
}


template <std::size_t N>
auto generate_nonce() -> apsn::result<std::string>
{
    unsigned char buf[N];
    int rc = RAND_bytes(buf, N);
    if (rc != 1) {
        return std::make_error_code(std::errc::protocol_error);
    }
    auto result = std::string{};
    for (auto v : buf) {
        result.append(fmt::format("{:02x}", v));
    }
    return result;
}


template <typename Traits>
auto apsn::http::digest_auth(request<Traits> & req, std::string const & realm)
    -> response
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::unauthorized, 
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");

    auto nonce = generate_nonce<16>();
    if (!nonce) {
        apsn::log::fatal("Unable to generate nonce!");
        std::terminate();
    }

    auto settings = fmt::format("Digest "
                "realm={}, "
                "nonce={}, "
                "charset=utf-8", 
            realm,
            *nonce
            );
    res.set("WWW-Authenticate", settings);
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
}






template <typename Traits>
detail::serve_string<Traits>::serve_string(string_fn<Traits> && func)
    : m_func{func}
{}


template <typename Traits>
auto detail::serve_string<Traits>::do_handle(request<Traits> & req) -> response
{
    auto content = m_func(req);
    const auto size = content.size();
    beast::http::response<beast::http::string_body> response{
        std::piecewise_construct,
        std::make_tuple(std::move(content)),
        std::make_tuple(beast::http::status::ok, req.version())};
        
    response.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(beast::http::field::content_type, "text/html; charset=utf-8");
    response.content_length(size);
    response.keep_alive(req.keep_alive());
    return response;
}




template <typename Traits>
detail::serve_json<Traits>::serve_json(json_fn<Traits> && func)
    : m_func{func}
{}


template <typename Traits>
auto detail::serve_json<Traits>::do_handle(request<Traits> & req) -> response
{
    auto content = m_func(req).dump();
    const auto size = content.size();
    beast::http::response<beast::http::string_body> response{
        std::piecewise_construct,
        std::make_tuple(content),
        std::make_tuple(beast::http::status::ok, req.version())};
        
    response.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(beast::http::field::content_type, "application/json");
    response.content_length(size);
    response.keep_alive(req.keep_alive());
    return response;
}



template <typename Traits>
detail::serve_files<Traits>::serve_files(fs::path root)
    : m_root{root}
{}


template <typename Traits>
auto detail::serve_files<Traits>::do_handle(request<Traits> & req) -> response
{
    auto method = req.method();
    auto version = req.version();
    auto target = req.target();
    auto keep_alive = req.keep_alive();

    // apsn::log::debug("http_session: {} {}", 
    //         req.method_string(),
    //         target);

    if (method != beast::http::verb::get) {
        return bad_request(req, "Unknown HTTP-method");
    }

    if (target.empty() ||
        target[0] != '/' ||
        target.find("..") != beast::string_view::npos)
    {
        return bad_request(req, "Illegal request-target");
    }

    fs::path path = m_root;
    path += std::string{target};

    if (target.back() == '/') {
        path /= "index.html";
    }

    // Attempt to open the file
    beast::error_code ec;
    beast::http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == boost::system::errc::no_such_file_or_directory) {
        apsn::log::error("Unable to find {}", path.string());
        return not_found(req);
    }

    // Handle an unknown error
    if (ec) {
        apsn::log::error("Unkown error: {}", ec.message());
        return server_error(req, ec.message());
    }

    // Cache the size since we need it after the move
    auto const size = body.size();

    beast::http::response<beast::http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(beast::http::status::ok, version)};
        
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, mime_type_for(path));
    res.content_length(size);
    res.keep_alive(keep_alive);
    return res;
}
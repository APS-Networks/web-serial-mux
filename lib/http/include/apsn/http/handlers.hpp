#pragma once

#include <apsn/logging.hpp>
#include <apsn/result.hpp>


#include <apsn/http/response.hpp>
#include <apsn/http/request.hpp>

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>


#include <openssl/err.h>
#include <openssl/rand.h>


#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace apsn::http {

namespace beast = boost::beast;
namespace fs = std::filesystem;

using beast::http::message_generator;

template <class Body, class Alloc>
using beast_request = beast::http::request<Body, 
    beast::http::basic_fields<Alloc>>;  

template <class Body, class Alloc>
using beast_response = beast::http::response<Body, 
    beast::http::basic_fields<Alloc>>;  

using beast_status = beast::http::status;



template <typename Traits>
using string_fn = std::function<std::string(request<Traits> &)>;

template <typename Traits>
using json_fn = std::function<nlohmann::json(request<Traits> &)>;

template <typename Traits>
using generator_fn = std::function<response(request<Traits> &)>;




auto mime_type_for(fs::path path) -> std::string;

template <typename Traits>
auto bad_request(request<Traits> & req, std::string_view why) -> response;

template <typename Traits>
auto not_found(request<Traits> & req) -> response;

template <typename Traits>
auto server_error(request<Traits> & req, std::string_view what) -> response;

template <typename Traits>
auto unauthorised(request<Traits> & req, 
        std::optional<fs::path> path = std::nullopt)
    -> response
{
    if (path) {
        beast::error_code ec;
        beast::http::file_body::value_type body;
        body.open(path->c_str(), beast::file_mode::scan, ec);

        // Handle the case where the file doesn't exist
        if (ec == boost::system::errc::no_such_file_or_directory) {
            apsn::log::error("Unable to find {}", path->string());
            return not_found(req);
        }

        // Handle an unknown error
        if (ec) {
            apsn::log::error("Unkown error: {}", ec.message());
            return server_error(req, ec.message());
        }

        auto const size = body.size();

        beast::http::response<beast::http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(beast::http::status::ok, req.version())};

        res.result(beast::http::status::unauthorized);
        res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.content_length(size);
        res.prepare_payload();
        return res;
    }

    beast::http::response<beast::http::string_body> res{
        beast::http::status::unauthorized, 
        req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "You are not authorised to view this content";
    res.prepare_payload();
    return res;
}

template <typename Traits>
auto basic_auth(request<Traits> & req, std::string_view realm) -> response;

template <typename Traits>
auto digest_auth(request<Traits> & req, std::string const & realm) -> response;



template <typename Traits>
class handler
{
public:
    using shared_type = typename Traits::shared_type;

    auto before_body(basic_parser & p, request<Traits> & req) -> std::optional<response>
    {
        return do_before_body(p, req);
    }

    auto handle(request<Traits> & req) -> response
    {
        return do_handle(req);
    }

    virtual auto name() const -> std::string_view = 0;
private:
    virtual auto do_handle(request<Traits> & req) -> response = 0;
    virtual auto do_before_body(basic_parser &, request<Traits> &) 
        -> std::optional<response>
    {
        /* default: noop */
        return std::nullopt;
    }
};


namespace detail {


template <template <typename> typename Impl, typename Traits>
class handler_base : public handler<Traits>
{
public:
    using handler<Traits>::handler;
    auto name() const -> std::string_view override
    {
        return Impl<Traits>::handler_name;
    }
};



template <typename Traits>
class serve_files : public handler_base<serve_files, Traits>
{
public:
    constexpr static auto handler_name = "serve_files";
    serve_files(fs::path root);
private:
    auto do_handle(request<Traits> & req) -> response override;
    fs::path m_root;
};

template <typename Traits>
class serve_string : public handler_base<serve_string, Traits>
{
public:
    constexpr static auto handler_name = "serve_string";
    serve_string(string_fn<Traits> && func);
private:
    auto do_handle(request<Traits> & req) -> response override;
    string_fn<Traits> m_func;
};


template <typename Traits>
class serve_json : public handler_base<serve_json, Traits>
{
public:
    constexpr static auto handler_name = "serve_json";
    serve_json(json_fn<Traits> && func);
private:
    auto do_handle(request<Traits> & req) -> response override;
    json_fn<Traits> m_func;
};


template <typename Traits>
class serve_generator : public handler_base<serve_generator, Traits>
{
public:
    constexpr static auto handler_name = "serve_generator";
    serve_generator(generator_fn<Traits> && func)
        : m_func{std::move(func)}
    {}
    auto do_handle(request<Traits> & req) -> response override
    { return m_func(req); }
private:
    generator_fn<Traits> m_func;
};


}


#include <apsn/http/detail/handlers.tpp>



template <typename Traits> 
auto serve_string(string_fn<Traits> && func) -> std::shared_ptr<handler<Traits>>
{
    return std::make_shared<detail::serve_string<Traits>>(std::move(func));
}

template <typename Traits> 
auto serve_json(json_fn<Traits> && func) -> std::shared_ptr<handler<Traits>>
{
    return std::make_shared<detail::serve_json<Traits>>(std::move(func));
}

template <typename Traits> 
auto serve_files(fs::path root) -> std::shared_ptr<handler<Traits>>
{
    return std::make_shared<detail::serve_files<Traits>>(root);
}

template <typename Traits> 
auto serve_generator(generator_fn<Traits> && func) -> std::shared_ptr<handler<Traits>>
{
    return std::make_shared<detail::serve_generator<Traits>>(std::move(func));
}



namespace detail {

template <typename T>
struct is_handler : std::false_type {};

template <template <typename> typename H, typename Traits>
struct is_handler<H<Traits>> : std::conditional_t<
        std::is_base_of_v<handler<Traits>, H<Traits>>,
        std::true_type,
        std::false_type
    > {};


template <typename T>
struct is_handler_ptr : std::false_type {};

template <typename T>
struct is_handler_ptr<std::shared_ptr<T>> : is_handler<T> {};

template <typename T>
constexpr static auto is_handler_ptr_v = is_handler_ptr<std::decay_t<T>>::value;

}


template <typename Traits, typename F>
auto ensure_handler(F && obj) -> std::shared_ptr<handler<Traits>>
{
    if constexpr (std::is_base_of_v<handler<Traits>, std::decay_t<F>>) {
        return std::make_shared<F>(std::forward<F>(obj));
    }
    else if constexpr (detail::is_handler_ptr_v<F>) {
        return obj;
    }
    else {
        using return_type = std::invoke_result_t<
                decltype(obj),
                request<Traits>&>;

        if constexpr (std::is_same_v<return_type, std::string>)
        {
            return serve_string<Traits>(std::forward<F>(obj));
        }
        else if constexpr (std::is_same_v<return_type, nlohmann::json>)
        {
            return serve_json<Traits>(std::forward<F>(obj));
        }
        else if constexpr (std::is_same_v<return_type, response>)
        {
            return serve_generator<Traits>(std::forward<F>(obj));
        }
    }
}

}

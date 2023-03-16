#pragma once

#include <apsn/http/handlers.hpp>

#include <apsn/http/detail/trie.hpp>

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <type_traits>



namespace apsn::http {


enum class router_match {
    exact,
    prefix
};

template <typename Alloc = std::allocator<char>>
using beast_empty_request = boost::beast::http::request<
        boost::beast::http::empty_body,
        boost::beast::http::basic_fields<Alloc>>;


template <typename Body, typename Alloc>
using beast_parser = boost::beast::http::request_parser<Body, Alloc>;

template <typename Alloc>
using beast_empty_parser = beast_parser<
        boost::beast::http::empty_body, Alloc>;

template <typename Traits>
class router
{
    using shared_type = typename Traits::shared_type;
public:
    router(std::shared_ptr<shared_type> shared);

    template <typename Alloc>
    auto before_body(std::string source, 
            beast_empty_parser<Alloc> & prsr,
            beast_empty_request<Alloc> & req)
        -> std::optional<response>;

    template <typename Body, typename Alloc>
    auto handle(std::string source, 
            beast_request<Body, Alloc> && req)-> response;

    auto handle_get(request<Traits> & request) -> response;

    auto handle_get_before_body(basic_parser & parser, request<Traits> & request)
        -> std::optional<response>;

    template <typename F>
    auto get(std::string path, router_match type, F && func);

private:
    struct matcher {
        router_match type;
        std::shared_ptr<handler<Traits>> handler_;
    };

    apsn::detail::lpm_map<std::string, matcher> m_get;
    std::shared_ptr<shared_type> m_shared;
};

#include <apsn/http/detail/router.tpp>

}
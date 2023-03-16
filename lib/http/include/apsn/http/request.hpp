#pragma once

#include <boost/beast.hpp>


#include <any>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace apsn::http {

enum class body_type
{
    string,
    file,
    empty,
    unknown
};



class basic_parser
{
    template <typename Body, typename Alloc>
    using beast_parser = boost::beast::http::request_parser<
            Body, Alloc>;

    struct interface {
        virtual auto body_limit(std::uint64_t limit) -> void = 0;
    };

    template <typename Body, typename Alloc>
    struct impl : interface
    {
        impl(beast_parser<Body, Alloc> & parser)
            : m_parser{parser}
        {}

        auto body_limit(std::uint64_t limit) -> void override
        { return m_parser.body_limit(limit); }

        beast_parser<Body, Alloc> & m_parser;
    };

public:
    template <typename Body, typename Alloc>
    basic_parser(beast_parser<Body, Alloc> & parser)
        : m_impl{std::make_shared<impl<Body, Alloc>>(parser)}
    {}

    auto body_limit(std::uint64_t limit) -> void
    { return m_impl->body_limit(limit); }

    std::shared_ptr<interface> m_impl;
};


class basic_request
{
    template <typename Alloc>
    using basic_fields = boost::beast::http::basic_fields<Alloc>;

    template <typename Body, typename Alloc = std::allocator<char>>
    using beast_request = boost::beast::http::request<Body, basic_fields<Alloc>>;

    using beast_verb        = boost::beast::http::verb;
    using beast_field       = boost::beast::http::field;
    using beast_string_body = boost::beast::http::string_body;
    using beast_file_body   = boost::beast::http::file_body;
    using beast_empty_body  = boost::beast::http::empty_body;

    using optional_string_body = 
        std::optional<std::reference_wrapper<std::string const>>;

    using optional_file_body = 
        std::optional<
            std::reference_wrapper<
                typename beast_file_body::value_type const>>;


public:
    enum class storage_category {
        reference,
        rvalue
    };

private:
    struct interface
    {
        virtual auto category() const -> storage_category = 0;

        virtual auto keep_alive() const -> bool = 0;
        virtual auto method() const -> beast_verb = 0;
        virtual auto method_string() const -> std::string_view = 0;
        virtual auto target() const -> std::string_view = 0;
        virtual auto version() const -> unsigned = 0;

        virtual auto operator[](beast_field field) const
            -> std::string_view = 0;

        virtual auto has_field(beast_field field) const -> bool = 0;

        virtual auto body_type() const -> body_type = 0;
        virtual auto string_body() const -> optional_string_body = 0;
        virtual auto file_body() const -> optional_file_body = 0;
    };



    template <storage_category Category, typename Body, typename Alloc>
    struct storage;

    template <typename Body, typename Alloc>
    struct storage<storage_category::reference, Body, Alloc>
    {
        using type = beast_request<Body, Alloc>&;
    };

    template <typename Body, typename Alloc>
    struct storage<storage_category::rvalue, Body, Alloc>
    {
        using type = beast_request<Body, Alloc>;
    };


    struct reference_tag_t {} reference_tag;
    struct rvalue_tag_t    {} rvalue_tag;


    template <storage_category Category, typename Body, typename Alloc>
    struct impl : interface
    {
        using value_type = typename Body::value_type;
        using stored_type = typename storage<Category, Body, Alloc>::type;

        impl(reference_tag_t, beast_request<Body, Alloc> & req)
            : m_request{req}
            , m_type{http::body_type::unknown}
        {
            set_body_type_();
        }

        impl(rvalue_tag_t, beast_request<Body, Alloc> && req)
            : m_request{std::move(req)}
            , m_type{http::body_type::unknown}
        {
            set_body_type_();
        }

        auto set_body_type_()
        {
            if constexpr (std::is_same_v<Body, beast_string_body>) {
                m_type = http::body_type::string;
            }
            else if constexpr (std::is_same_v<Body, beast_string_body>) {
                m_type = http::body_type::file;
            }
            else if constexpr (std::is_same_v<Body, beast_empty_body>) {
                m_type = http::body_type::empty;
            }
        }

        auto category() const -> storage_category
        { return Category; }


        auto keep_alive() const -> bool
        { return m_request.keep_alive(); }


        auto method() const -> beast_verb
        { return m_request.method(); }

        auto method_string() const -> std::string_view override
        { return m_request.method_string(); }

        auto target() const -> std::string_view override
        { return m_request.target(); }

        auto version() const -> unsigned override
        { return m_request.version(); }
        

        auto has_field(beast_field field) const -> bool override
        { return m_request.find(field) != std::end(m_request); }

        auto operator[](beast_field field) const -> std::string_view
        { return m_request[field]; }




        auto body_type() const -> http::body_type override
        { return m_type; }

        auto string_body() const -> optional_string_body
        { 
            if constexpr (std::is_same_v<Body, beast_string_body>) {
                return m_request.body();
            }
            else {
                return std::nullopt;
            }
        }

        auto file_body() const -> optional_file_body
        { 
            if constexpr (std::is_same_v<Body, beast_file_body>) {
                return m_request.body();
            }
            else {
                return std::nullopt;
            }
        }

        stored_type m_request;
        http::body_type m_type;
    };


public:
    template <typename Body, typename Alloc>
    using impl_ref = impl<storage_category::reference, Body, Alloc>;

    template <typename Body, typename Alloc>
    using impl_rvalue = impl<storage_category::rvalue, Body, Alloc>;

    template <typename Body, typename Alloc>
    basic_request(std::string source, 
            beast_request<Body, Alloc> & req)
        : m_source{source}
        , m_impl{std::make_unique<impl_ref<Body, Alloc>>(reference_tag, req)}
    {}

    template <typename Body, typename Alloc>
    basic_request(std::string source, 
            beast_request<Body, Alloc> && req)
        : m_source{source}
        , m_impl{std::make_unique<impl_rvalue<Body, Alloc>>(
                rvalue_tag, std::move(req))}
    {}


    auto category() const -> storage_category
    { return m_impl->category(); }

    /* Message methods */
    auto keep_alive() const -> bool
    { return m_impl->keep_alive(); }


    /* Header methods */
    auto method() const -> beast_verb
    { return m_impl->method(); }

    auto method_string() const -> std::string_view
    { return m_impl->method_string(); }

    auto target() const -> std::string_view
    { return m_impl->target(); }

    auto version() const -> unsigned
    { return m_impl->version(); }


    /* Field methods */
    auto has_field(beast_field field) const -> bool
    { return m_impl->has_field(field); }

    auto operator[](beast_field field) const -> std::string_view
    { return m_impl->operator[](field); }


    /* Body methods */
    auto body_type() const -> http::body_type
    { return m_impl->body_type(); }

    auto string_body() const -> optional_string_body
    { return m_impl->string_body(); }

    auto file_body() const -> optional_file_body
    { return m_impl->file_body(); }


    /* Metadata */
    auto has_meta(std::string key) const -> bool
    { return m_data.contains(key); }

    template <typename T>
    auto get_meta(std::string key) -> T&
    { return std::any_cast<T&>(m_data[key]); }

    template <typename T>
    auto set_meta(std::string key, T && value) -> void
    { m_data[key] = value; }


    auto source() const -> std::string const &
    { return m_source; }


private:
    std::string m_source;
    std::unique_ptr<interface> m_impl;
    std::map<std::string, std::any> m_data;

};


template <typename Traits>
class request : public basic_request
{
    using shared_type = typename Traits::shared_type;
    using unique_type = typename Traits::unique_type;
public:
    template <typename Body, typename Alloc = std::allocator<char>>
    using beast_request = boost::beast::http::request<Body,
            basic_fields<Alloc>>;

    template <typename Body, typename Alloc>
    request(std::string source,
            beast_request<Body, Alloc> && req,
            std::shared_ptr<shared_type> shared)
        : basic_request{source, std::move(req)}
        , m_shared{shared}
    {}

    template <typename Body, typename Alloc>
    request(std::string source,
            beast_request<Body, Alloc> & req,
            std::shared_ptr<shared_type> shared)
        : basic_request{source, req}
        , m_shared{shared}
    {}

    /* Shared data */
    auto shared() -> std::shared_ptr<shared_type>
    { return m_shared; }

    auto session() -> std::shared_ptr<unique_type>
    { return m_unique; }
private:
    std::shared_ptr<shared_type> m_shared;
    std::shared_ptr<unique_type> m_unique;
};


}
#pragma once

#include <boost/beast.hpp>


#include <any>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace apsn::http {


class response
{
    template <typename Alloc>
    using basic_fields = boost::beast::http::basic_fields<Alloc>;

    template <typename Body, typename Alloc = std::allocator<char>>
    using beast_response = boost::beast::http::response<Body, basic_fields<Alloc>>;

    using beast_verb        = boost::beast::http::verb;
    using beast_field       = boost::beast::http::field;
    using beast_string_body = boost::beast::http::string_body;
    using beast_file_body   = boost::beast::http::file_body;
    using beast_status      = boost::beast::http::status;
    using beast_message     = boost::beast::http::message_generator;

    using optional_string_body = 
        std::optional<std::reference_wrapper<std::string const>>;

    using optional_file_body = 
        std::optional<
            std::reference_wrapper<
                typename beast_file_body::value_type const>>;

    struct interface
    {
        virtual auto status() const -> beast_status = 0;
        virtual auto size() const -> std::size_t = 0;
        virtual auto message() -> beast_message = 0;

        virtual auto operator[](beast_field field) const
            -> std::string_view = 0;

        virtual auto operator[](std::string_view field) const
            -> std::string_view = 0;

        virtual auto has_field(std::string_view field) const -> bool = 0;
        virtual auto has_field(beast_field field) const -> bool = 0;

        virtual auto insert(std::string_view key, std::string_view value)
            -> void = 0;

        virtual auto insert(beast_field key, std::string_view value)
            -> void = 0;
    };

    template <typename Body, typename Alloc>
    struct impl : interface
    {
        impl(beast_response<Body, Alloc> && req)
            : m_response{std::move(req)}
        {}

        auto status() const -> beast_status override
        { return m_response.result(); }

        auto size() const -> std::size_t override
        { 
            auto sz = m_response.payload_size();
            return sz ? *sz : 0;
        }

        auto operator[](std::string_view field) const -> std::string_view
        { return m_response[field]; }

        auto operator[](beast_field field) const -> std::string_view
        { return m_response[field]; }

        auto has_field(std::string_view field) const -> bool override
        { return m_response.find(field) != std::end(m_response); }

        auto has_field(beast_field field) const -> bool override
        { return m_response.find(field) != std::end(m_response); }

        auto message() -> beast_message override
        { return beast_message{std::move(m_response)}; }

        auto insert(std::string_view key, std::string_view value)
            -> void override
        { return m_response.insert(key, value); }

        auto insert(beast_field key, std::string_view value)
            -> void override
        { return m_response.insert(key, value); }

        beast_response<Body, Alloc> m_response;
    };


public:
    template <typename Body, typename Alloc>
    response(beast_response<Body, Alloc> && req)
        : m_impl{std::make_unique<impl<Body, Alloc>>(std::move(req))}
    {}

    auto status() const -> beast_status 
    { return m_impl->status(); };

    auto size() const -> std::size_t 
    { return m_impl->size(); };

    auto message() -> beast_message
    { return m_impl->message(); };

    auto has_field(std::string_view field) const -> bool
    { return m_impl->has_field(field); }

    auto has_field(beast_field field) const -> bool
    { return m_impl->has_field(field); }

    auto operator[](std::string_view field) const -> std::string_view
    { return m_impl->operator[](field); }

    auto operator[](beast_field field) const -> std::string_view
    { return m_impl->operator[](field); }

    auto insert(std::string_view key, std::string_view value) -> void
    { return m_impl->insert(key, value); }

    auto insert(beast_field key, std::string_view value) -> void
    { return m_impl->insert(key, value); }

private:
    std::unique_ptr<interface> m_impl;
};

} /* namesapce apsn::http */
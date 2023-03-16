#pragma once

#include <apsn/http/handlers.hpp>
#include <apsn/http/headers.hpp>
#include <apsn/http/response.hpp>

#include <boost/beast.hpp>
#include <md5.h>


#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace apsn::http::middleware {

template <typename Traits>
class base : public handler<Traits>
{
public:
    base(std::shared_ptr<handler<Traits>> next)
        : m_next{next}
    {}

    auto next() -> handler<Traits> &
    {
        return *m_next;
    }

    auto do_before_body(basic_parser & p, request<Traits> & req)
        -> std::optional<response> override
    {
        return m_next->before_body(p, req);
    }

    auto do_handle(request<Traits> & req) -> response override
    {
        return m_next->handle(req);
    }

private:
    std::shared_ptr<handler<Traits>> m_next;
};


namespace detail {


template <template <typename> typename Impl, typename Traits>
class middleware_base : public base<Traits>
{
public:
    using base<Traits>::base;

    auto name() const -> std::string_view override
    {
        return Impl<Traits>::handler_name;
    }
};


template <typename Traits>
class xclacks_overhead : public middleware_base<xclacks_overhead, Traits>
{
public:
    constexpr static auto handler_name = "xclacks_overhead";

    xclacks_overhead(std::shared_ptr<handler<Traits>> next)
        : middleware_base<xclacks_overhead, Traits>{next}
        , m_value{"GNU Terry Pratchett"}
    {}
    xclacks_overhead(std::string value, std::shared_ptr<handler<Traits>> next)
        : middleware_base<xclacks_overhead, Traits>{next}
        , m_value{value}
    {}

    auto do_handle(request<Traits> & req) -> response
    {
        auto response = this->next().handle(req);
        if (!response.has_field("X-Clacks-Overhead")) {
            response.insert("X-Clacks-Overhead", m_value);
        }
        return response;
    }

private:
    std::string m_value;
};


template <typename Traits>
class ncsa_logger : public middleware_base<ncsa_logger, Traits>
{
    using shared_type = typename Traits::shared_type;

public:
    using middleware_base<ncsa_logger, Traits>::middleware_base;

    constexpr static auto handler_name = "ncsa_logger";

    auto do_handle(request<Traits> & req) -> response
    {
        auto response = this->next().handle(req);
        if (req.has_meta("username")) {
            apsn::log::info("{} - {} \"{} {} HTTP/{}.{}\" {} {}",
                    req.source(),
                    req.template get_meta<std::string>("username"),
                    req.method_string(),
                    req.target(),
                    req.version()/10,
                    req.version()%10,
                    static_cast<unsigned>(response.status()),
                    response.size());
        }
        else {
            apsn::log::info("{} - - \"{} {} HTTP/{}.{}\" {} {}",
                    req.source(),
                    req.method_string(),
                    req.target(),
                    req.version()/10,
                    req.version()%10,
                    static_cast<unsigned>(response.status()),
                    response.size());
        }
        return response;
    }
};


template <typename Traits>
class digest_auth : public middleware_base<digest_auth, Traits>
{
    using shared_type = typename Traits::shared_type;
public:

    constexpr static auto handler_name = "digest_auth";

    digest_auth(std::string const & realm,
            std::string const & ha1,
            std::shared_ptr<handler<Traits>> next)
        : middleware_base<digest_auth, Traits>{next}
        , m_realm{realm}
        , m_ha1{ha1}
    {
    }

    auto do_before_body(basic_parser & p, request<Traits> & req)
            -> std::optional<response>
    {
        using apsn::http::headers::authorisation;
        using beast_field = boost::beast::http::field;

        if (req.has_field(beast_field::authorization)) {
            auto auth = authorisation::parse(req[beast_field::authorization]);
            if (!auth) {
                return apsn::http::bad_request(req, auth.error.message());
            }

            if (auth->has_field(authorisation::field::username) && 
                auth->has_field(authorisation::field::realm) && 
                auth->has_field(authorisation::field::uri) && 
                auth->has_field(authorisation::field::nonce) && 
                auth->has_field(authorisation::field::response))
            {
                auto expected = MD5{fmt::format("{}:{}:{}",
                    m_ha1, 
                    auth->get(authorisation::field::nonce),
                    MD5{fmt::format("{}:{}", 
                            req.method_string(),
                            auth->get(authorisation::field::uri)
                        )}.hexdigest()
                    )}.hexdigest();

                if (expected == auth->get(authorisation::field::response))
                {
                    req.set_meta("username", 
                        auth->get(authorisation::field::username));
                    p.body_limit(std::numeric_limits<std::uint64_t>::max());
                    return this->next().before_body(p, req); 
                }
            }   
        }
        return apsn::http::digest_auth(req, m_realm);
    }

private:
    std::string m_realm;
    std::string m_ha1;
};

}


template <typename Traits, typename F>
auto xclacks_overhead(F && next) -> std::shared_ptr<base<Traits>>
{
    return std::make_shared<detail::xclacks_overhead<Traits>>(
            ensure_handler<Traits>(std::forward<F>(next)));
}

template <typename Traits, typename F>
auto xclacks_overhead(std::string value, F && next) -> std::shared_ptr<base<Traits>>
{
    return std::make_shared<detail::xclacks_overhead<Traits>>(
            ensure_handler<Traits>(value, std::forward<F>(next)));
}

template <typename Traits, typename F>
auto ncsa_logger(F && next) -> std::shared_ptr<base<Traits>>
{
    return std::make_shared<detail::ncsa_logger<Traits>>(
            ensure_handler<Traits>(std::forward<F>(next)));
}

template <typename Traits, typename F>
auto digest_auth(std::string const & realm,
            std::string const & ha1,
            F && next) -> std::shared_ptr<base<Traits>>
{
    return std::make_shared<detail::digest_auth<Traits>>(realm,
            ha1,
            ensure_handler<Traits>(std::forward<F>(next)));
}

}
#pragma once

#include "cli/base_state.hpp"
#include "cli/control_state.hpp"

#include <apsn/http/headers.hpp>
#include <apsn/http/request.hpp>
#include <apsn/http/websocket.hpp>

namespace smux {


template <typename Traits, bool IsSSL>
class cli_handler
    : public std::enable_shared_from_this<cli_handler<Traits, IsSSL>>
    , public apsn::ws::websocket_handler<cli_handler, Traits, IsSSL>
{
    using base_type = apsn::ws::websocket_handler<cli_handler, Traits, IsSSL>;

public:
    template <typename Body, typename Alloc>
    using beast_request = beast::http::request<Body,
            beast::http::basic_fields<Alloc>>;


    using stream_type = typename base_type::stream_type;
    using unique_type = typename base_type::unique_type;
    using shared_type = typename base_type::shared_type;

    cli_handler(stream_type && stream,
            std::shared_ptr<unique_type> unique,
            std::shared_ptr<shared_type> shared)
        : base_type{std::move(stream), unique, shared}
    {
        this->stream().binary(true);
    }


    ~cli_handler()
    {
        apsn::log::trace("cli_handler::~cli_handler");
        m_state->cancel();
        this->shared()->sessions.unregister_session(this);
    }

    auto cancel() -> void override
    {
        apsn::log::trace("cli_handler::cancel");
        base_type::cancel();
    }
    
    template <typename Body, typename Alloc>
    auto handle_request(beast_request<Body, Alloc> & req)
        -> std::optional<apsn::http::response>
    {
        using namespace std::string_literals;
        using apsn::http::headers::authorisation;
        using apsn::http::request;
        using beast_field = beast::http::field;

        auto source = beast::get_lowest_layer(this->stream())
                    .socket()
                    .remote_endpoint()
                    .address()
                    .to_string();
                
        auto user = "<unknown>"s;

        if (req.find(beast_field::authorization) != std::end(req)) {
            auto auth = authorisation::parse(req[beast_field::authorization]);
            if (auth->has_field(authorisation::field::username)) {
                user = auth->get(authorisation::field::username);
            }
        }
        else {
            auto areq = request<Traits>(source, std::move(req), this->shared());
            return apsn::http::unauthorised(areq);
        }

        this->shared()->sessions.register_session(this, 
                user,
                beast::get_lowest_layer(this->stream())
                    .socket()
                    .remote_endpoint()
                    .address()
                    .to_string()
                );
        
        m_state = std::make_shared<cli::control_state>(this, this->shared());
        return {};
    }

    auto handle_accept() -> void
    {
        m_state->run();
        this->shared()->sessions.set_state(this, m_state->name());
    }

    auto handle_message() -> void
    {
        auto & buf = this->buffer();
        for (auto it = asio::buffers_begin(buf.data()); 
            it != asio::buffers_end(buf.data());
            ++it)
        {
            auto next_state = m_state->feed(*it);
            if (next_state) {
                apsn::log::debug("New state: {}", m_state->name());
                m_state->cancel();
                m_state = std::move(next_state);
                m_state->run();
                this->shared()->sessions.set_state(this, m_state->name());
                break;
            }
        }
    }

    std::shared_ptr<cli::base_state> m_state;
};



}
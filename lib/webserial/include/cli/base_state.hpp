#pragma once

#include "context.hpp"

#include <apsn/ansi.hpp>
#include <apsn/logging.hpp>

#include <fmt/format.h>

#include <map>
#include <memory>
#include <vector>


namespace apsn::ws {
// template <typename Traits>
class websocket_base;
}


namespace smux::cli {


enum class input_state {
    text,
    esc,
    csi,
    dcs,
    apc
};

auto to_string(input_state state) -> std::string;
auto operator<<(std::ostream& lhs, input_state rhs) -> std::ostream&;

}


template <>
struct fmt::formatter<smux::cli::input_state>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(smux::cli::input_state value, FormatContext& ctx) const
            -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", to_string(value));
    }
};




namespace smux::cli {


class base_state
{
public:
    base_state(apsn::ws::websocket_base * session,
            std::shared_ptr<smux::context> ctx);

    auto feed(char c) -> std::shared_ptr<base_state>;
    auto get_state() const -> input_state;

    virtual auto run() -> void {}
    virtual auto cancel() -> void {}
    virtual auto name() const -> std::string = 0;

    virtual ~base_state()
    {
        apsn::log::trace("base_state::~base_state");
    }

protected:
    template <typename ... Args>
    auto write_session(fmt::format_string<Args...> format, Args && ... args)
        -> void
    {
        m_out << fmt::format(format, std::forward<Args>(args)...);
    }

    auto write_session(std::string const & message) -> void
    {
        m_out << message;
    }

    auto transition_to(input_state new_state) -> void ;

    /* Common stuff */
    apsn::ws::websocket_base * m_session;
    std::ostream & m_out;
    std::shared_ptr<smux::context> m_ctx;

private:
    auto feed_esc(char c) -> std::shared_ptr<base_state>;
    auto feed_text(char c) -> std::shared_ptr<base_state>;
    auto feed_apc(char c) -> std::shared_ptr<base_state>;
    auto feed_dcs(char c) -> std::shared_ptr<base_state>;
    auto feed_csi(char c) -> std::shared_ptr<base_state>;

    auto string_terminated() const -> bool;

    /* Message contains both prefix and intermediate bytes. Maybe 0 length. */
    virtual auto on_csi(std::string message, apsn::ansi::csi_final final) 
        -> std::shared_ptr<base_state> = 0;

    virtual auto on_apc(std::string message)
        -> std::shared_ptr<base_state> = 0;

    /* This might be broken somewhat -- use APC for custom messages instead*/
    virtual auto on_dcs(std::string message)
        -> std::shared_ptr<base_state> = 0;

    virtual auto on_char(char c) 
        -> std::shared_ptr<base_state> = 0;


    std::vector<char> m_inbuf;
    std::vector<char> m_cmdbuf;
    input_state m_state;
};

}
#include "cli/base_state.hpp"

#include <apsn/http/websocket.hpp>


#include "context.hpp"

#include <apsn/ansi.hpp>

#include <apsn/logging.hpp>

#include <fmt/format.h>

#include <iostream>
#include <map>
#include <memory>
#include <vector>





auto smux::cli::to_string(input_state state) -> std::string
{
    auto mapped = std::map<input_state, std::string> {
        { input_state::text, "input_state::text" },
        { input_state::esc,  "input_state::esc"  },
        { input_state::csi,  "input_state::csi"  },
        { input_state::dcs,  "input_state::dcs"  },
        { input_state::apc,  "input_state::apc"  }
    };
    return mapped.at(state);
}


auto smux::cli::operator<<(std::ostream& lhs, input_state rhs)
    -> std::ostream&
{
    return lhs << to_string(rhs);
}


using smux::cli::base_state;


base_state::base_state(
        apsn::ws::websocket_base * session,
        std::shared_ptr<smux::context> ctx)
    : m_session{session}
    , m_out{m_session->ostream()}
    , m_ctx{ctx}
    , m_inbuf{}
    , m_cmdbuf{}
    , m_state{input_state::text}
{}


auto base_state::transition_to(input_state new_state) -> void 
{
    apsn::log::debug("Transition from {} to {}", m_state, new_state);
    m_state = new_state;
}


auto base_state::feed(char c) -> std::shared_ptr<base_state>
{
    switch (m_state) {
    case input_state::text: return feed_text(c);
    case input_state::esc: return feed_esc(c);
    case input_state::csi: return feed_csi(c);
    case input_state::dcs: return feed_dcs(c);
    case input_state::apc: return feed_apc(c);
    default: return nullptr;
    }
}


auto base_state::feed_esc(char c) -> std::shared_ptr<base_state>
{
    namespace ansi = apsn::ansi;
    
    switch (ansi::fe_cast(c)) {
    case ansi::fe::CSI:
        transition_to(input_state::csi); break;
    case ansi::fe::DCS:
        transition_to(input_state::dcs); break;
    case ansi::fe::APC:
        transition_to(input_state::apc); break;
    default:
        apsn::log::warn("Unexpected ESC character: {}", c);
        transition_to(input_state::text);
    }
    return nullptr;
}


auto base_state::feed_text(char c) -> std::shared_ptr<base_state>
{
    namespace ansi = apsn::ansi;
    switch (ansi::c0_cast(c)) {
    case ansi::c0::ESC:
        transition_to(input_state::esc);
        return nullptr;
    default:
        return on_char(c);
    }
}


/* Note that XTerm does not define or accept any APC sequences and is 
   defined as `ESC _ ${str} ST`, but DEC STD 070 defines it as
   `ESC_ I..I F ${str} ST`, where:

        ESC_: command introducer
        I:    intermediates (zero or more)
        F:    final character
        ST:   string terminator
*/
auto base_state::feed_apc(char c) -> std::shared_ptr<base_state>
{
    m_cmdbuf.push_back(c);
    if (string_terminated()) {
        m_cmdbuf.pop_back();
        m_cmdbuf.pop_back();
        auto msg = std::string{m_cmdbuf.begin(), m_cmdbuf.end()};
        m_cmdbuf.clear();
        transition_to(input_state::text);
        return on_apc(msg);
    }

    return nullptr;
}


auto base_state::feed_dcs(char c) -> std::shared_ptr<base_state>
{
    m_cmdbuf.push_back(c);
    if (string_terminated()) {
        m_cmdbuf.pop_back();
        m_cmdbuf.pop_back();
        auto msg = std::string{m_cmdbuf.begin(), m_cmdbuf.end()};
        m_cmdbuf.clear();
        transition_to(input_state::text);
        return on_dcs(msg);
    }
    return nullptr;
}


auto base_state::feed_csi(char c) -> std::shared_ptr<base_state>
{
    if (c >= 0x40 && c <= 0x7e) {
        /* This is a "final" */
        auto msg = std::string{m_cmdbuf.begin(), m_cmdbuf.end()};
        m_cmdbuf.clear();
        transition_to(input_state::text);
        return on_csi(msg, static_cast<apsn::ansi::csi_final>(c));
    }
    else {
        m_cmdbuf.push_back(c);
    }
    return nullptr;
}


auto base_state::string_terminated() const -> bool
{
    if (m_cmdbuf.size() < 2) {
        return false;
    }
    auto last_two = std::string{m_cmdbuf.cend() - 2, m_cmdbuf.cend()};
    return last_two == "\x1b\\";
}


auto base_state::get_state() const -> input_state
{ return m_state; }
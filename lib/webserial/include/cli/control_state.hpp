#pragma once

#include "history.hpp"


#include "context.hpp"
#include "strings.hpp"
#include "base_state.hpp"

#include <apsn/ansi.hpp>
#include <apsn/logging.hpp>

#include <apsn/http/websocket.hpp>

#include <cli/cli.h>


#include <list>
#include <vector>


namespace smux::cli {




class control_state
    : public base_state
    , public ::cli::SessionBase
{
public:
    control_state(apsn::ws::websocket_base * session,
            std::shared_ptr<smux::context> ctx);

    ~control_state();
    
    auto name() const -> std::string override
    { return "control"; }

    /* CLI Session */
    auto Current(::cli::Menu * menu) -> void override
    { m_current_menu = menu; }

    auto OutStream() -> std::ostream& override
    { return this->m_out; };

private:
    auto on_csi(std::string message, apsn::ansi::csi_final final)
        -> std::shared_ptr<base_state> override;

    auto on_dcs(std::string message)
        -> std::shared_ptr<base_state> override;

    auto on_apc(std::string message)
        -> std::shared_ptr<base_state> override;

    auto on_char(char c) -> std::shared_ptr<base_state> override ;

    auto run() -> void override;
    // template <typename ... Args>
    // auto write(fmt::format_string<Args...> format, Args && ... args) -> void
    // {
    //     m_out << fmt::format(format, std::forward<Args>(args)...);
    // }

    // auto write(std::string const & message) -> void
    // {
    //     m_out << message;
    // }

    auto help() -> void;
    auto flush() -> void;
    auto make_menus() -> void;
    auto prompt() -> void;
    auto send_error(std::string message) -> void;
    auto execute() -> std::shared_ptr<base_state>;

    /* Base stuff */

    history m_history;

    /* Command line interface */
    std::unique_ptr<::cli::Cli> m_cli;
    std::unique_ptr<::cli::Menu> m_global_menu;
    ::cli::Menu * m_current_menu;

    /* This could be changed in a CLI callback which needs to be 
       checked against once the handler has returned. */
    std::shared_ptr<base_state> m_next_state;

    /* For building up commands */
    std::vector<char> m_inbuf;

};

}
#pragma once

#include "control_state.hpp"

#include "base_state.hpp"
#include "context.hpp"
#include "port.hpp"

#include <apsn/http/websocket.hpp>

#include <apsn/ansi.hpp>

#include <boost/asio/serial_port.hpp>

#include <array>

namespace smux::cli {

class serial_state
    : public base_state
    , public std::enable_shared_from_this<serial_state>
{
public:
    using boost_serial_port = boost::asio::serial_port;


    serial_state(apsn::ws::websocket_base * session,
            std::shared_ptr<context> ctx,
            port & port_info,
            boost_serial_port && serial_port);

    ~serial_state();
    
    auto name() const -> std::string override
    { return "serial"; }

    auto run() -> void override;
    auto cancel() -> void override;

private:
    template <typename ... Args>
    auto write_serial(fmt::format_string<Args...> format, Args && ... args) -> void
    {
        auto str = fmt::format(format, std::forward<Args>(args)...);
        auto sp = std::make_shared<std::string>(std::move(str));
        return write_serial(sp);
    }

    auto write_serial(std::shared_ptr<std::string const> const& ss) -> void;
    auto fail(sys::error_code ec, std::string extra) -> void;

    auto on_read(beast::error_code ec, std::size_t bytes_transferred) -> void;
    auto on_write(beast::error_code ec, std::size_t bytes_transferred) -> void;
    auto on_send(std::shared_ptr<std::string const> const& ss) -> void;
    auto on_csi(std::string message, apsn::ansi::csi_final final)
        -> std::shared_ptr<base_state> override;

    auto on_dcs(std::string message)
        -> std::shared_ptr<base_state> override;

    auto on_apc(std::string message)
        -> std::shared_ptr<base_state> override;

    auto on_char(char c) -> std::shared_ptr<base_state> override;

    port & m_info;
    boost_serial_port m_port;
    std::array<char, 256> m_buffer;
    std::vector<std::shared_ptr<std::string const>> m_send_queue;
};

}
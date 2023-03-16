#include "cli/serial_state.hpp"

#include "cli/base_state.hpp"
// #include <apsn/smux/detail/boost.hpp>

#include "context.hpp"
#include "port.hpp"

#include <apsn/http/websocket.hpp>

#include <apsn/ansi.hpp>
#include <apsn/fmt.hpp>


#include <boost/asio/serial_port.hpp>

#include <array>


using smux::cli::serial_state;

serial_state::serial_state(apsn::ws::websocket_base * session, 
        std::shared_ptr<context> ctx,
        port & port_info,
        boost_serial_port && port)
    : base_state{session, ctx}
    , m_info{port_info}
    , m_port{std::move(port)}
    , m_buffer{}
{
    apsn::log::trace("serial_state::serial_state");

}


serial_state::~serial_state() {
    apsn::log::trace("serial_state::~serial_state");
    // auto sess_lock = m_ctx->sessions.lock();
    // auto port_lock = m_ctx->ports.lock();
    m_ctx->sessions.set_device(m_session, "");
    m_info.in_use = false;
}


auto serial_state::run() -> void
{
    namespace ansi = apsn::ansi;

    apsn::log::trace("serial_state::run");
    
    write_session(ansi::send_dcs("serial", 'S'));

    write_session("\x1b[2J");   /* Clear remote screen     */
    write_session("\x1b[0;0H"); /* Send cursor to top left */
    write_session("\x1b]2;Serial Port on {} @ {}\x1b\\",
        m_info.device,
        m_info.options.baud_rate.value());

    /* TODO: don't type "connected" until a serial link has actually
                been established. */
    write_session("Connected.\r\nType Ctrl + q to exit\r\n");


    auto self = shared_from_this();
    m_port.async_read_some(asio::buffer(m_buffer.data(), m_buffer.size()), 
            [self](sys::error_code ec, std::size_t len){
                self->on_read(ec, len);
            });

    // write_serial("{}", ansi::c0::FF);

}


auto serial_state::cancel() -> void
{
    apsn::log::trace("serial_state::cancel");
    m_port.cancel();
}

auto serial_state::fail(sys::error_code ec,
        std::string extra) -> void
{
    namespace ansi = apsn::ansi;

    apsn::log::error("{}{}serial_state:{} {} {}",
            ansi::italic,
            ansi::bold, 
            ansi::reset,
            ec.message(),
            extra);
}



auto serial_state::write_serial(std::shared_ptr<std::string const> const& ss) -> void
{
    apsn::log::trace("serial_state::send {}", *ss);
    
    auto self = shared_from_this();
    asio::post(
        m_ctx->ioc,
        beast::bind_front_handler(
            &serial_state::on_send,
            shared_from_this(),
            ss));
}


auto serial_state::on_read(beast::error_code ec, std::size_t bytes_transferred) -> void
{
    apsn::log::trace("serial_state::on_read {}", bytes_transferred);

    if (ec) {
        return fail(ec, "read");
    }

    auto to_write = std::string{m_buffer.data(), 
            m_buffer.data() + bytes_transferred};

    m_out << to_write;

    auto self = shared_from_this();
    m_port.async_read_some(asio::buffer(m_buffer.data(), m_buffer.size()), 
            [self](sys::error_code ec, std::size_t len){
                self->on_read(ec, len);
            });
}


auto serial_state::on_write(beast::error_code ec, 
        [[maybe_unused]]std::size_t bytes_transferred) -> void
{
    apsn::log::trace("serial_state::on_write {}", bytes_transferred);
    if (ec) {
        return fail(ec, "write");
    }

    m_send_queue.erase(m_send_queue.begin());

    if (!m_send_queue.empty()) {
        auto self = shared_from_this();
        m_port.async_write_some(asio::buffer(&m_send_queue.front(), 1),
            [self](sys::error_code ec, std::size_t len){
                self->on_write(ec, len);
            });
    }
}


auto serial_state::on_send(std::shared_ptr<std::string const> const& ss) -> void
{
    m_send_queue.push_back(ss);

    if (m_send_queue.size() > 1) {
        return;
    }

    auto self = shared_from_this();
    m_port.async_write_some(asio::buffer(*m_send_queue.front()),
        [self](sys::error_code ec, std::size_t len){
            self->on_write(ec, len);
        });
}


auto serial_state::on_csi(std::string message, apsn::ansi::csi_final final)
    -> std::shared_ptr<base_state>
{
    apsn::log::trace("serial_state::on_csi, message: '{}', final: '{}'", message,
        apsn::ansi::to_value(final));

    write_serial("\x1b[{}{}", message, apsn::ansi::to_value(final));

    return nullptr;
}


auto serial_state::on_dcs(std::string message)
    -> std::shared_ptr<base_state>
{
    apsn::log::trace("serial_state::on_dcs {}", message);
    return nullptr;
}


auto serial_state::on_apc(std::string message)
    -> std::shared_ptr<base_state>
{
    apsn::log::trace("serial_state::on_apc {}", message);
    return nullptr;
}


auto serial_state::on_char(char c) -> std::shared_ptr<base_state> 
{
    namespace ansi = apsn::ansi;

    apsn::log::trace("serial_state::on_char {}", c);
    switch (ansi::c0_cast(c)) {
    case ansi::c0::DC1: {
        return std::make_shared<control_state>(m_session, m_ctx);
    }
    default: 
        write_serial("{}", c);
    }
    return nullptr;
}